#include <i86.h>

#include "int33_mouse.h"

static void call( union REGS &r ) {
  int86( 0x33, &r, &r );
}

Int33Mouse::Int33Mouse() : present_( false ), wheel_( false ) {}

bool Int33Mouse::detect() {
  union REGS r;
  r.w.ax = 0;
  call( r );
  if ( r.w.ax != 0xFFFF ) return false;
  present_ = true;
  r.w.ax = 0x11;
  call( r );
  wheel_ = ( r.w.ax == 0x574D && ( r.h.cl & 1 ) );
  return true;
}

bool Int33Mouse::present() const {
  return present_;
}

bool Int33Mouse::hasWheel() const {
  return wheel_;
}

void Int33Mouse::reset() {
  if ( !present_ ) return;
  union REGS r;
  r.w.ax = 0;
  call( r );
}

void Int33Mouse::show() {
  if ( !present_ ) return;
  union REGS r;
  r.w.ax = 1;
  call( r );
}

void Int33Mouse::hide() {
  if ( !present_ ) return;
  union REGS r;
  r.w.ax = 2;
  call( r );
}

void Int33Mouse::poll( MouseState &state ) {
  union REGS r;
  r.w.ax = 3;
  call( r );
  state.wheel = (signed char)r.h.bh;
  state.row = r.w.dx >> 3;
  state.col = r.w.cx >> 3;
  state.buttons = r.h.bl;
}

unsigned Int33Mouse::leftPresses() {
  union REGS r;
  r.w.ax = 5;
  r.w.bx = 0;
  call( r );
  return r.w.bx;
}
