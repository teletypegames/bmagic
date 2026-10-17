#!/usr/bin/env python3
"""Screen tests for BMAGIC.EXE in DOSBox.

The test version of the program (build.sh test) reads its keystrokes from
TESTKEYS.TXT in its folder. Each case copies a sample boot drive of test/drives to a
C: drive, with the program in C:\\APPS\\BMAGIC, runs it in DOSBox with the keystrokes
of the case, and compares the result with test/expected/<case>.txt: the text screen the
program saved (SCREEN.BIN) when the keystrokes ran out, followed by the boot files that
are new or changed on the drive and the files of its backups.

usage: run.py [--update] [case ...]
  --update   write the results to expected/ instead of comparing
DOSBOX=<path> selects the emulator (default: dosbox-staging / dosbox in PATH,
or DOSBox Staging.app on macOS). JOBS=<n> runs that many cases at once
(default: 4).
"""
import os
import re
import shutil
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
SOURCE = os.path.dirname(HERE)
BOOT_FILES = ("CONFIG.SYS", "AUTOEXEC.BAT", "MSDOS.SYS")
CP437_LOW = " ☺☻♥♦♣♠•◘○◙♂♀♪♫☼►◄↕‼¶§▬↨↑↓→←∟↔▲▼"
NAMED_KEYS = {
    "Enter": 0x1C0D, "Esc": 0x011B, "Tab": 0x0F09, "Bksp": 0x0E08,
    "Up": 0x4800, "Down": 0x5000, "Left": 0x4B00, "Right": 0x4D00,
    "PgUp": 0x4900, "PgDn": 0x5100, "Home": 0x4700, "End": 0x4F00,
    "Ins": 0x5200, "Del": 0x5300, "CtrlPgUp": 0x8400, "CtrlPgDn": 0x7600, "CtrlY": 0x1519,
    **{f"F{n}": (0x3A + n) << 8 for n in range(1, 11)},
}


def find_dosbox():
    if os.environ.get("DOSBOX"):
        return os.environ["DOSBOX"]
    for name in ("dosbox-staging", "dosbox"):
        path = shutil.which(name)
        if path:
            return path
    app = "/Applications/DOSBox Staging.app/Contents/MacOS/dosbox"
    if os.path.exists(app):
        return app
    sys.exit("DOSBox not found (set DOSBOX=<path>)")


def parse_keys(text):
    """'Down Enter "abc" 0x3B00' -> [0x5000, 0x1C0D, 0x61, 0x62, 0x63, 0x3B00]"""
    keys = []
    for tok in re.findall(r'"[^"]*"|\S+', text):
        if tok.startswith('"'):
            keys += [ord(c) for c in tok[1:-1]]
        elif tok in NAMED_KEYS:
            keys.append(NAMED_KEYS[tok])
        else:
            keys.append(int(tok, 0))
    return keys


def load_cases():
    """cases.txt: name | keys of run 1 ; keys of run 2 ... | sample drive (default: menu)"""
    cases = []
    with open(os.path.join(HERE, "cases.txt"), encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            parts = [p.strip() for p in line.split("|")]
            runs = [parse_keys(r) for r in parts[1].split(";")]
            drive = parts[2] if len(parts) > 2 and parts[2] else "menu"
            cases.append((parts[0], runs, drive))
    return cases


def render(screen):
    rows = []
    for r in range(25):
        chars = screen[r * 160:(r + 1) * 160:2]
        rows.append("".join(CP437_LOW[c] if c < 32 else bytes([c]).decode("cp437")
                            for c in chars).rstrip())
    return "\n".join(rows).rstrip("\n") + "\n"


def normalize(text):
    text = re.sub(r"\d{4}-\d\d-\d\d", "YYYY-MM-DD", text)
    return re.sub(r"\b\d\d:\d\d\b", "HH:MM", text)


def build():
    """the test version, into SOURCE/build"""
    subprocess.run(["sh", os.path.join(SOURCE, "build.sh"), "test"], check=True,
                   stdout=subprocess.DEVNULL)
    return os.path.join(SOURCE, "build")


def run_dosbox(dosbox, cdrive):
    version = subprocess.run([dosbox, "--version"], capture_output=True, text=True).stdout
    cmds = [f"mount c {cdrive}", "c:", r"\APPS\BMAGIC\BMAGIC.EXE C:", "exit"]
    cmd = [dosbox]
    if "staging" in version.lower():
        cmd += ["--noprimaryconf", "--nolocalconf"]
    else:
        conf = os.path.join(cdrive, "..", "test.conf")
        with open(conf, "w") as f:
            f.write("[sdl]\nautolock=false\n[cpu]\ncycles=max\n")
        cmd += ["-conf", conf]
    for c in cmds:
        cmd += ["-c", c]
    env = {**os.environ, "SDL_VIDEODRIVER": "dummy", "SDL_AUDIODRIVER": "dummy"}
    subprocess.run(cmd, env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                   timeout=60)


def text_of(path):
    return open(path, "rb").read().decode("cp437").replace("\r\n", "\n")


def written_files(cdrive, drive):
    """the boot files that are new or changed, then the files of the backups"""
    out = ""
    for name in BOOT_FILES:
        path = os.path.join(cdrive, name)
        orig = os.path.join(HERE, "drives", drive, name)
        if not os.path.exists(path):
            continue
        if os.path.exists(orig) and open(orig, "rb").read() == open(path, "rb").read():
            continue
        out += f"{name}:\n" + text_of(path)
    backup = os.path.join(cdrive, "APPS", "BMAGIC", "BACKUP")
    for folder, _, names in sorted(os.walk(backup)):
        for name in sorted(names):
            rel = os.path.relpath(os.path.join(folder, name), cdrive).replace(os.sep, "\\")
            out += f"{rel}:\n" + text_of(os.path.join(folder, name))
    return out


def run_case(dosbox, tmp, built, name, runs, drive):
    cdrive = os.path.join(tmp, name, "c")
    prog = os.path.join(cdrive, "APPS", "BMAGIC")
    os.makedirs(prog)
    for f in os.listdir(os.path.join(HERE, "drives", drive)):
        shutil.copy(os.path.join(HERE, "drives", drive, f), cdrive)
    shutil.copy(os.path.join(built, "BMAGIC.EXE"), prog)
    result = ""
    for keys in runs:               # the runs share the drive and the backups
        screen = os.path.join(prog, "SCREEN.BIN")
        if os.path.exists(screen):
            os.remove(screen)
        with open(os.path.join(prog, "TESTKEYS.TXT"), "w") as f:
            f.write(" ".join(f"{k:04X}" for k in keys) + "\r\n")
        run_dosbox(dosbox, cdrive)
        result = render(open(screen, "rb").read()) if os.path.exists(screen) else "(the program ended)\n"
    for f in ("TESTKEYS.TXT", "SCREEN.BIN"):
        if os.path.exists(os.path.join(prog, f)):
            os.remove(os.path.join(prog, f))
    return normalize(result + written_files(cdrive, drive))


def main():
    argv = sys.argv[1:]
    update = "--update" in argv
    selected = [a for a in argv if a != "--update"]
    dosbox = find_dosbox()
    cases = [c for c in load_cases() if not selected or c[0] in selected]
    built = build()
    failed = 0
    jobs = max(1, int(os.environ.get("JOBS") or 4))
    with tempfile.TemporaryDirectory() as tmp, ThreadPoolExecutor(jobs) as pool:
        # every case has its own folder in tmp; the results come in the order of the cases
        results = pool.map(lambda case: run_case(dosbox, tmp, built, *case), cases)
        for (name, *_), got in zip(cases, results):
            path = os.path.join(HERE, "expected", name + ".txt")
            if update:
                with open(path, "w", encoding="utf-8") as f:
                    f.write(got)
                print(f"updated  {name}")
                continue
            want = open(path, encoding="utf-8").read() if os.path.exists(path) else None
            if got == want:
                print(f"ok       {name}")
            else:
                failed += 1
                print(f"FAIL     {name}")
                with open(os.path.join(HERE, "expected", name + ".got"), "w",
                          encoding="utf-8") as f:
                    f.write(got)
    if not update:
        print(f"{len(cases) - failed}/{len(cases)} passed")
        if failed:
            print("(the results of the failed cases are in expected/*.got)")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
