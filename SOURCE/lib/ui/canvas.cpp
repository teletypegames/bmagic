#include "canvas.h"
#include "../core/str.h"

Canvas::Canvas( Screen &screen ) : screen_( screen ) {}

Screen &Canvas::screen() {
  return screen_;
}

void Canvas::flush() {
  screen_.flush();
}

int Canvas::at( int row, int col ) {
  return row * Cols + col;
}

int Canvas::put( int pos, char c, unsigned char attr ) {
  screen_.put( pos, c, attr );
  return pos + 1;
}

int Canvas::text( int pos, const char *s, int max, unsigned char attr ) {
  while ( max-- > 0 && *s ) pos = put( pos, *s++, attr );
  return pos;
}

int Canvas::textAt( int row, int col, const char *s, int max, unsigned char attr ) {
  return text( at( row, col ), s, max, attr );
}

int Canvas::fillTo( int pos, int end, unsigned char attr ) {
  while ( pos < end ) pos = put( pos, ' ', attr );
  return pos;
}

int Canvas::number( int pos, unsigned n, unsigned char attr ) {
  char digits[6];
  numStr( digits, n );
  return text( pos, digits, 6, attr );
}

void Canvas::field( int col, int row, const char *s, unsigned char attr, int width ) {
  int pos = at( row, col );
  for ( int i = 0; i < width && col + i < Cols; i++ ) pos = put( pos, *s ? *s++ : ' ', attr );
}

void Canvas::boxLine( int row, int left, int right, char l, char fill, char r, unsigned char attr ) {
  int pos = put( at( row, left ), l, attr );
  for ( int i = left + 1; i < right; i++ ) pos = put( pos, fill, attr );
  put( pos, r, attr );
}

void Canvas::box( int top, int left, int bottom, int right, unsigned char attr ) {
  boxLine( top, left, right, (char)0xC9, (char)0xCD, (char)0xBB, attr );
  for ( int row = top + 1; row < bottom; row++ ) boxLine( row, left, right, (char)0xBA, ' ', (char)0xBA, attr );
  boxLine( bottom, left, right, (char)0xC8, (char)0xCD, (char)0xBC, attr );
}

void Canvas::separator( int row, int left, int right, unsigned char attr ) {
  boxLine( row, left, right, (char)0xC7, (char)0xC4, (char)0xB6, attr );
}

int Canvas::boxTitle( int row, int col, const char *s, int max, unsigned char attr ) {
  int pos = put( at( row, col ), ' ', attr );
  pos = text( pos, s, max, attr );
  return put( pos, ' ', attr );
}

int Canvas::footer( int pos, const char *s ) {
  unsigned char attr = 0x70;
  for ( ; *s; s++ ) {
    if ( *s == '~' ) attr ^= 0x04;
    else pos = put( pos, *s, attr );
  }
  return pos;
}
