#include <i86.h>

#include "dos_screen.h"

DosScreen::DosScreen( Mode mode )
  : mode_( mode ), mono_( false ), video_( (unsigned short __far *)MK_FP( 0xB800, 0 ) ) {}

void DosScreen::setTextMode() {
  union REGS r;
  r.w.ax = 3;
  int86( 0x10, &r, &r );
}

void DosScreen::begin() {
  if ( mode_ == SetMode ) {
    setTextMode();
    return;
  }
  union REGS r;
  r.h.ah = 0x0F;
  int86( 0x10, &r, &r );
  mono_ = ( r.h.al == 7 );
  video_ = (unsigned short __far *)MK_FP( mono_ ? 0xB000 : 0xB800, 0 );
}

void DosScreen::end() {
  if ( mode_ == SetMode ) {
    setTextMode();
    return;
  }
  for ( int pos = 0; pos < Cells; pos++ ) put( pos, ' ', 0x07 );
  union REGS r;
  r.h.ah = 0x01;
  r.w.cx = mono_ ? 0x0B0C : 0x0607;
  int86( 0x10, &r, &r );
  moveCursor( 0, 0 );
}

void DosScreen::put( int pos, char c, unsigned char attr ) {
  if ( pos < 0 || pos >= Cells ) return;
  if ( mono_ ) attr = ( ( attr & 0xF0 ) && attr != 0x0F ) ? 0x70 : 0x07;
  video_[pos] = ( (unsigned short)attr << 8 ) | (unsigned char)c;
}

unsigned DosScreen::cell( int pos ) const {
  return video_[pos];
}

void DosScreen::flush() {}

void DosScreen::hideCursor() {
  union REGS r;
  r.h.ah = 0x01;
  r.w.cx = 0x2000;
  int86( 0x10, &r, &r );
}

void DosScreen::moveCursor( int row, int col ) {
  union REGS r;
  r.h.ah = 0x02;
  r.h.bh = 0;
  r.h.dh = (unsigned char)row;
  r.h.dl = (unsigned char)col;
  int86( 0x10, &r, &r );
}
