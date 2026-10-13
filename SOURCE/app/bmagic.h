/*
   BMAGIC.EXE - Boot Magic: the boot menu of CONFIG.SYS and AUTOEXEC.BAT (and the startup
   options of a Windows 95/98 MSDOS.SYS) in the root folder of the boot drive.
   Launch: BMAGIC [X:]     X: another drive than the boot drive
   The backups go to BACKUP\ in the program's folder.
   Exit code: 0 = files written, 1 = nothing written, 2 = error.
*/

#ifndef BMAGIC_APP_BMAGIC_H
#define BMAGIC_APP_BMAGIC_H

class BootSystem;
class FileSystem;
class Keyboard;
class Screen;
class Speaker;
class TextOutput;

/* What BMAGIC gets from the machine it runs on (main_bmagic.cpp builds it). */
struct Machine {
  FileSystem &fs;
  Screen &screen;
  Keyboard &keyboard;
  Speaker &speaker;
  TextOutput &out;
  BootSystem &system;
};

/* progDir: the program's folder, with a trailing '\' */
int runBmagic( Machine &machine, const char *progDir, int argc, char *argv[] );

#endif
