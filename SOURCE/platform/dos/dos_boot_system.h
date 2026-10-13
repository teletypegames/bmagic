#ifndef BMAGIC_PLATFORM_DOS_BOOT_SYSTEM_H
#define BMAGIC_PLATFORM_DOS_BOOT_SYSTEM_H

#include "../boot_system.h"

/* INT 21h 3305h for the boot drive, the Open Watcom C library for the rest. */
class DosBootSystem : public BootSystem {
public:
  char bootDrive();
  bool setAttributes( const char *path, unsigned attrib );
  void now( unsigned &date, unsigned &time );
};

#endif
