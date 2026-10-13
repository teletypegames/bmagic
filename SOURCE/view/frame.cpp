#include <stddef.h>
#include <string.h>

#include "frame.h"
#include "../lib/ui/canvas.h"

enum { Cols = Canvas::Cols };
static const unsigned char AttrBar = 0x70;
static const unsigned char AttrStatus = 0x1E;
static const unsigned char AttrError = 0x4F;
static const unsigned char AttrEdit = 0x70;

Frame::Frame( Canvas &canvas ) : canvas_( canvas ) {}

Canvas &Frame::canvas() {
  return canvas_;
}

void Frame::title( const char *left, const char *right ) {
  int pos = canvas_.put( 0, ' ', AttrBar );
  pos = canvas_.text( pos, left, 60, AttrBar );
  int len = right ? strlen( right ) : 0;
  pos = canvas_.fillTo( pos, Cols - 1 - len, AttrBar );
  if ( right ) pos = canvas_.text( pos, right, len, AttrBar );
  canvas_.fillTo( pos, Cols, AttrBar );
}

void Frame::keys( const char *keys ) {
  int pos = canvas_.put( Canvas::at( KeysRow, 0 ), ' ', AttrBar );
  pos = canvas_.footer( pos, keys );
  canvas_.fillTo( pos, Canvas::at( KeysRow + 1, 0 ), AttrBar );
}

void Frame::status( const char *text, unsigned char attr ) {
  int pos = canvas_.put( Canvas::at( StatusRow, 0 ), ' ', attr );
  if ( text ) pos = canvas_.text( pos, text, Cols - 2, attr );
  canvas_.fillTo( pos, Canvas::at( StatusRow + 1, 0 ), attr );
}

void Frame::message( const char *text ) {
  status( text, text ? AttrStatus : 0x17 );
  canvas_.screen().hideCursor();
}

void Frame::error( const char *text ) {
  status( text, AttrError );
  canvas_.screen().hideCursor();
}

int Frame::editWidth( const char *label ) {
  return Cols - 3 - strlen( label );
}

void Frame::editLine( const char *label, const char *text, int scroll, int pos ) {
  int at = canvas_.put( Canvas::at( StatusRow, 0 ), ' ', AttrStatus );
  at = canvas_.text( at, label, 20, AttrStatus );
  at = canvas_.put( at, ' ', AttrStatus );
  int first = at, width = editWidth( label );
  int len = strlen( text );
  for ( int i = 0; i < width; i++ ) at = canvas_.put( at, scroll + i < len ? text[scroll + i] : ' ', AttrEdit );
  canvas_.fillTo( at, Canvas::at( StatusRow + 1, 0 ), AttrStatus );
  canvas_.screen().moveCursor( StatusRow, first - Canvas::at( StatusRow, 0 ) + pos - scroll );
}

void Frame::flush() {
  canvas_.flush();
}
