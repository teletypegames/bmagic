#include <dos.h>
#include <i86.h>

#include "dos_boot_system.h"

char DosBootSystem::bootDrive() {
  union REGS r;
  r.w.ax = 0x3305;
  r.h.dl = 0;
  intdos( &r, &r );
  return ( r.h.dl >= 1 && r.h.dl <= 26 ) ? (char)( 'A' + r.h.dl - 1 ) : 'C';
}

bool DosBootSystem::setAttributes( const char *path, unsigned attrib ) {
  return _dos_setfileattr( path, attrib ) == 0;
}

void DosBootSystem::now( unsigned &date, unsigned &time ) {
  struct dosdate_t d;
  struct dostime_t t;
  _dos_getdate( &d );
  _dos_gettime( &t );
  date = ( ( d.year - 1980 ) << 9 ) | ( d.month << 5 ) | d.day;
  time = ( t.hour << 11 ) | ( t.minute << 5 ) | ( t.second / 2 );
}
