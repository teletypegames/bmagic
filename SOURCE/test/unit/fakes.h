/*
   The test doubles of BMAGIC's own machine interfaces, and texts to start from.
*/

#ifndef BMAGIC_TEST_FAKES_H
#define BMAGIC_TEST_FAKES_H

#include <string.h>

#include "../../lib/platform/speaker.h"
#include "../../lib/platform/text_output.h"
#include "../../platform/boot_system.h"

class FakeBootSystem : public BootSystem {
public:
  FakeBootSystem() : drive( 'C' ), setCount( 0 ), lastAttrib( 0 ) { lastPath[0] = 0; }
  char bootDrive() { return drive; }
  bool setAttributes( const char *path, unsigned attrib ) {
    setCount++;
    strcpy( lastPath, path );
    lastAttrib = attrib;
    return true;
  }
  void now( unsigned &date, unsigned &time ) {
    date = ( ( 2026 - 1980 ) << 9 ) | ( 10 << 5 ) | 13;
    time = ( 14 << 11 ) | ( 5 << 5 );
  }

  char drive;
  int setCount;
  char lastPath[80];
  unsigned lastAttrib;
};

class CountingSpeaker : public Speaker {
public:
  CountingSpeaker() : beeps( 0 ) {}
  void beep() { beeps++; }
  int beeps;
};

class CapturedOutput : public TextOutput {
public:
  CapturedOutput() { text[0] = 0; }
  void write( const char *s ) { strcat( text, s ); }
  char text[400];
};

/* A CONFIG.SYS / AUTOEXEC.BAT pair with a menu of two items. */
static const char MenuConfig[] = "[MENU]\r\n"
                                 "MENUITEM=NORMAL,Normal start\r\n"
                                 "MENUITEM=GAMES,Games with EMS\r\n"
                                 "MENUDEFAULT=NORMAL,10\r\n"
                                 "\r\n"
                                 "[COMMON]\r\n"
                                 "DOS=HIGH,UMB\r\n"
                                 "FILES=40\r\n"
                                 "\r\n"
                                 "[NORMAL]\r\n"
                                 "DEVICE=C:\\DOS\\HIMEM.SYS\r\n"
                                 "\r\n"
                                 "[GAMES]\r\n"
                                 "DEVICE=C:\\DOS\\HIMEM.SYS\r\n"
                                 "DEVICE=C:\\DOS\\EMM386.EXE RAM\r\n";
static const char MenuAutoexec[] = "@ECHO OFF\r\n"
                                   "PROMPT $P$G\r\n"
                                   "GOTO %CONFIG%\r\n"
                                   ":NORMAL\r\n"
                                   "SMARTDRV\r\n"
                                   "GOTO END\r\n"
                                   ":GAMES\r\n"
                                   "MOUSE\r\n"
                                   "GOTO END\r\n"
                                   ":END\r\n";
/* A plain pair without a menu. */
static const char PlainConfig[] = "DEVICE=C:\\DOS\\HIMEM.SYS\r\nDOS=HIGH\r\nFILES=30\r\n";
static const char PlainAutoexec[] = "@ECHO OFF\r\nPATH C:\\DOS\r\n";

#endif
