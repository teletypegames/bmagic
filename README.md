# doslib

The code that the `dos/` programs ([DOSINST](https://git.teletypegames.org/dos/dosinst), [GMENU](https://git.teletypegames.org/dos/gmenu)) share, and the rules their code follows. Each program has it in its `SOURCE/lib` folder as a git subtree, so it builds on its own (also in CI):

```sh
# in the program's repository: take the newest doslib
git subtree pull --prefix=SOURCE/lib ssh://git@git.teletypegames.org:2222/dos/doslib master --squash
# a change made in SOURCE/lib goes back to doslib
git subtree push --prefix=SOURCE/lib ssh://git@git.teletypegames.org:2222/dos/doslib master
```

Its own unit tests run on the host:

```sh
tools/host_test.sh test/build/unit core/*.cpp net/*.cpp ui/*.cpp platform/file_system.cpp platform/file_tree.cpp \
  platform/text_output.cpp testing/scripted_keyboard.cpp testing/screen_dump.cpp testing/host/*.cpp test/*.cpp
```

## Folders

| Folder | What |
|---|---|
| `core` | No I/O at all: strings and 8.3 names (`str`), INI files (`ini`), UTF-8 and code page 437 (`text`), CRC-32, `Error`, `NameList`, `Progress`, shell sort. |
| `platform` | The interfaces of the machine: `Screen`, `Keyboard`, `Mouse`, `Clock`, `Idle`, `FileSystem`, `Network`, `Speaker`, `TextOutput`, and helpers on them (`readFile`, `makeFolders`, `removeTree`). |
| `platform/dos` | Their DOS implementations (video memory, BIOS, INT 33h, the Open Watcom C library; `DosFileSystem` also serves the Win32 builds). |
| `platform/win32` | Their Win32 console implementations. |
| `net` | The FTP client on `Network`, and the byte sinks of the downloads. |
| `ui` | Drawing on a `Screen` (`Canvas`), the BIOS key codes (`keys.h`), `ListCursor`. |
| `testing` | The hooks of the test builds: `ScriptedKeyboard` (keys from a text), `dumpScreen` (SCREEN.BIN). `testing/host`: test doubles and a small test framework for the unit tests on the host. |
| `test` | The unit tests of doslib. |
| `tools` | `watcom.sh` (sourced by the build scripts), `check_layers.py`, `host_test.sh`, `format.sh`. |

## Layers

A program is built in layers, each using only the ones below it, and only through interfaces:

```
main_*.cpp       the machine: picks the platform implementations, calls app/
app/             puts the layers together (the composition root, a heap object)
controller/      keys and commands -> service calls; returns how a page ends
view/            draws; changes nothing
service/         what the program does
repository/      interfaces of the storage; repository/<kind>/ implements them
model/           the data and its rules, no I/O
platform/        the program's own machine interfaces; platform/dos, platform/win32
lib/             this library
```

`tools/check_layers.py` checks it on every build in CI: which folder may include which (a `folder/*.h` rule means only the interfaces right in that folder, not the implementations below it), that the DOS and Windows headers (`dos.h`, `io.h`, `windows.h`, ...) are only in the platform implementations and `main_*.cpp`, that there is no `goto`, and no `exit()` outside `main_*.cpp`.

Everything above the platform compiles on the host too, so it is unit tested there (`tools/host_test.sh`, each program's `SOURCE/test/unit/run.sh`), on the test doubles of `testing/host`.

## The code

- **C++98, the part Open Watcom compiles**, for `wpp` (16-bit DOS), `wpp386` (Win32) and the host's compiler alike: classes, pure virtual interfaces, references, simple templates. No exceptions, no RTTI, no STL, no `iostream`, nothing of C++11.
- **An interface** is a class with pure virtual methods only, named by what it is (`FileSystem`, `GameRepository`); an implementation is named by how it does it (`DosFileSystem`, `FolderGameRepository`, `FakeFileSystem`). Interfaces have no destructor: objects are never deleted through them, and on 16 bits a virtual or protected one makes every program bigger.
- **Dependencies come in through the constructor**, as references; only `app/` and `main_*.cpp` create the concrete objects.
- **Failures** are a `bool` (or a small enum) and an `Error` that the layers share: the failing layer sets it, the one that started the operation shows it.
- **No global state** outside the platform implementations (a flag an interrupt handler sets) and `main_*.cpp`.
- **Memory**: big buffers are members of the objects `app/` allocates once, or come from `main_*.cpp`; nothing allocates in a loop.
- **Names**: files `snake_case.cpp/.h`, types `PascalCase`, functions and variables `camelCase`, members `name_`, constants `PascalCase` (`enum` or `static const`, no `#define`).
- **Format**: `tools/format.sh` with `.clang-format` (clang-format 18): `f( x )`, `if ( x ) y;`, 120 columns. CI checks it.
- **Comments**: a header says what its interface promises (what it returns, when it fails), and a file format is described where it is read and written; the code itself is named well enough not to need narrating. Why a change was made goes into its commit message.

## Open Watcom notes

- The 16-bit compiler of the large model crashes on a module name longer than 25 characters (its code segment is named after it): `watcom.sh`'s `compile` cuts it to 24 with `-nm`.
- A static object over 32 KB goes into an initialized far segment, that is, into the EXE file. Big objects are allocated on the heap (`app/`), big buffers are plain arrays.
- `-zm` puts every function into a segment of its own, so `option eliminate` drops the ones a program does not use.
- `printf` and the stdio streams cost several KB in a 16-bit program: `Error` and `printTo` use them, so only link them where they are needed (`DosTextOutput` writes without stdio).
- Watcom looks for a quoted include in the current folder before the including file's folder: no two headers of a program may have the same name.
