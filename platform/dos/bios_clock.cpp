#include <dos.h>
#include <i86.h>

#include "bios_clock.h"

unsigned long BiosClock::ms() {
  return *(unsigned long __far *)MK_FP( 0x40, 0x6C ) * 55ul;
}

unsigned BiosClock::ticks() {
  return *(unsigned __far *)MK_FP( 0x40, 0x6C );
}

unsigned BiosClock::today() {
  struct dosdate_t d;
  _dos_getdate( &d );
  return ( ( d.year - 1980 ) << 9 ) | ( d.month << 5 ) | d.day;
}
