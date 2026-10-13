#!/usr/bin/env python3
"""Checks the layers of a program's SOURCE folder (default: the current folder).

Every #include "..." of a layer has to point into one of the folders the layer may use
(a "folder/*.h" entry means only the headers right in that folder: the interfaces, not
the implementations in its subfolders). The system headers of DOS and Windows are only
for the platform implementations and main_*.cpp; goto is nowhere, exit() only in
main_*.cpp (and the test hooks of lib/testing). Vendored code (MTCP) and the tests are
not checked.
"""
import fnmatch
import os
import re
import sys

ANY = ["*"]
RULES = [
    ("lib/core/", ["lib/core/"]),
    ("lib/platform/dos/", ["lib/platform/", "lib/core/"]),
    ("lib/platform/win32/", ["lib/platform/", "lib/core/"]),
    ("lib/platform/", ["lib/platform/*.h", "lib/core/"]),
    ("lib/net/", ["lib/net/", "lib/core/", "lib/platform/*.h"]),
    ("lib/ui/", ["lib/ui/", "lib/core/", "lib/platform/*.h"]),
    ("lib/testing/", ["lib/testing/", "lib/core/", "lib/platform/*.h", "lib/ui/"]),
    ("lib/tools/", []),
    ("model/", ["model/", "lib/core/"]),
    ("repository/*/", ["repository/", "model/", "lib/core/", "lib/net/", "lib/platform/*.h"]),
    ("repository/", ["repository/*.h", "model/", "lib/core/"]),
    ("service/", ["service/", "repository/*.h", "model/", "lib/core/", "lib/net/", "lib/platform/*.h", "platform/*.h"]),
    ("controller/", ["controller/", "service/", "view/", "model/", "lib/core/", "lib/ui/", "lib/platform/*.h"]),
    ("view/", ["view/", "model/", "lib/core/", "lib/ui/", "lib/platform/*.h"]),
    ("platform/dos/", ["platform/", "lib/"]),
    ("platform/win32/", ["platform/", "lib/"]),
    ("platform/", ["platform/*.h", "lib/core/"]),
    ("app/", ["app/", "controller/", "service/", "repository/", "view/", "model/", "lib/core/", "lib/net/",
              "lib/ui/", "lib/platform/*.h", "platform/*.h"]),
    ("main_", ANY),
]
PLATFORM_HEADERS = {"dos.h", "i86.h", "io.h", "direct.h", "bios.h", "conio.h", "fcntl.h", "sys/stat.h",
                    "windows.h", "winsock.h"}
PLATFORM_DIRS = ("lib/platform/dos/", "lib/platform/win32/", "platform/dos/", "platform/win32/", "main_")
SKIP = ("MTCP/", "test/", "build/", "lib/test/")


def layer_of(path):
    for pattern, allowed in RULES:
        if fnmatch.fnmatch(path, pattern + "*") or path.startswith(pattern):
            return pattern, allowed
    return None, None


def may_use(target, allowed):
    for a in allowed:
        if a == "*":
            return True
        if a.endswith("*.h"):
            if os.path.dirname(target) + "/" == a[:-3] and target.endswith(".h"):
                return True
        elif target.startswith(a):
            return True
    return False


def code_of(text):
    """The text without comments and strings."""
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    text = re.sub(r"//[^\n]*", " ", text)
    return re.sub(r'"(\\.|[^"\\])*"', '""', text)


def check(root):
    problems = []
    for folder, subs, names in os.walk(root):
        for name in sorted(names):
            if not name.endswith((".cpp", ".h")):
                continue
            path = os.path.relpath(os.path.join(folder, name), root).replace(os.sep, "/")
            if path.startswith(SKIP):
                continue
            text = open(os.path.join(folder, name), encoding="latin-1").read()
            layer, allowed = layer_of(path)
            if layer is None:
                problems.append(f"{path}: not in a layer")
                continue
            for n, line in enumerate(text.splitlines(), 1):
                m = re.match(r'\s*#\s*include\s*"([^"]+)"', line)
                if m:
                    target = os.path.normpath(os.path.join(folder, m.group(1)))
                    if not os.path.exists(target):
                        continue
                    target = os.path.relpath(target, root).replace(os.sep, "/")
                    if not may_use(target, allowed):
                        problems.append(f"{path}:{n}: {layer} may not include {target}")
                m = re.match(r"\s*#\s*include\s*<([^>]+)>", line)
                if m and m.group(1) in PLATFORM_HEADERS and not path.startswith(PLATFORM_DIRS):
                    problems.append(f"{path}:{n}: <{m.group(1)}> only in the platform implementations")
            code = code_of(text)
            if re.search(r"\bgoto\b", code):
                problems.append(f"{path}: goto")
            if re.search(r"(?<![\w:.])exit\s*\(", code) and not path.startswith(("main_", "lib/testing/")):
                problems.append(f"{path}: exit() outside main")
    return problems


def main():
    root = os.path.realpath(sys.argv[1] if len(sys.argv) > 1 else ".")
    problems = check(root)
    for p in problems:
        print(p)
    print(f"layers: {'ok' if not problems else str(len(problems)) + ' problems'}")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
