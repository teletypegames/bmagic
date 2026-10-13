#ifndef DOSLIB_PLATFORM_CONSOLE_SCREEN_H
#define DOSLIB_PLATFORM_CONSOLE_SCREEN_H

#include <windows.h>

#include "../screen.h"

/* The 80x25 screen in a console screen buffer of its own, which end() gives back. */
class ConsoleScreen : public Screen {
public:
  ConsoleScreen();
  void begin();
  void end();
  void put( int pos, char c, unsigned char attr );
  unsigned cell( int pos ) const;
  void flush();
  void hideCursor();
  void moveCursor( int row, int col );

private:
  HANDLE screen_;
  HANDLE oldScreen_;
  CHAR_INFO cells_[Cells];
  int dirtyFrom_, dirtyTo_;
};

#endif
