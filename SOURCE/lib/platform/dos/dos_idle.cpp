#include <i86.h>

#include "dos_idle.h"

void DosIdle::idle() {
  union REGS r;
  r.w.ax = 0x1680;
  int86( 0x2F, &r, &r );
}
