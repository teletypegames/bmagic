/*
   The text screen in video memory. SetMode: begin() and end() set mode 3 (80x25 color,
   which also clears it). KeepMode: the screen stays as it is (a monochrome card works
   too: the colored bars become inverse); end() clears it and puts the cursor back.
*/

#ifndef DOSLIB_PLATFORM_DOS_SCREEN_H
#define DOSLIB_PLATFORM_DOS_SCREEN_H

#include "../screen.h"

class DosScreen : public Screen {
public:
  enum Mode { SetMode, KeepMode };

  explicit DosScreen( Mode mode );
  void begin();
  void end();
  void put( int pos, char c, unsigned char attr );
  unsigned cell( int pos ) const;
  void flush();
  void hideCursor();
  void moveCursor( int row, int col );

private:
  void setTextMode();

  Mode mode_;
  bool mono_;
  unsigned short __far *video_;
};

#endif
