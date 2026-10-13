/*
   The 80x25 text screen. A position is a cell index (row * 80 + column); a cell has a
   code page 437 character and a PC color attribute. What is put may only be visible
   after flush().
*/

#ifndef DOSLIB_PLATFORM_SCREEN_H
#define DOSLIB_PLATFORM_SCREEN_H

class Screen {
public:
  enum { Cols = 80, Rows = 25, Cells = Cols * Rows };

  /* Takes the screen over, and gives it back as it found it (cleared). */
  virtual void begin() = 0;
  virtual void end() = 0;
  virtual void put( int pos, char c, unsigned char attr ) = 0;
  /* attribute << 8 | character */
  virtual unsigned cell( int pos ) const = 0;
  virtual void flush() = 0;
  virtual void hideCursor() = 0;
  virtual void moveCursor( int row, int col ) = 0;
};

#endif
