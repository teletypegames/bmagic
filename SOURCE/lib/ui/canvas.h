/*
   Drawing on a Screen. Positions are cell indexes (row * 80 + column); the drawing calls
   return the position after what they drew.
*/

#ifndef DOSLIB_UI_CANVAS_H
#define DOSLIB_UI_CANVAS_H

#include "../platform/screen.h"

class Canvas {
public:
  enum { Cols = Screen::Cols, Rows = Screen::Rows };

  explicit Canvas( Screen &screen );
  Screen &screen();
  void flush();

  static int at( int row, int col );
  int put( int pos, char c, unsigned char attr );
  /* At most max characters of s. */
  int text( int pos, const char *s, int max, unsigned char attr );
  int textAt( int row, int col, const char *s, int max, unsigned char attr );
  int fillTo( int pos, int end, unsigned char attr );
  int number( int pos, unsigned n, unsigned char attr );
  /* s in a field of width cells, padded with spaces, cut at the right edge. */
  void field( int col, int row, const char *s, unsigned char attr, int width );

  /* A double-line frame, a separator line inside one, " text " on a frame. */
  void box( int top, int left, int bottom, int right, unsigned char attr );
  void separator( int row, int left, int right, unsigned char attr );
  int boxTitle( int row, int col, const char *s, int max, unsigned char attr );
  /* Key help on the gray footer: '~' toggles the highlight color. */
  int footer( int pos, const char *s );

private:
  void boxLine( int row, int left, int right, char l, char fill, char r, unsigned char attr );

  Screen &screen_;
};

#endif
