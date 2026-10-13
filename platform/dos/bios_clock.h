#ifndef DOSLIB_PLATFORM_BIOS_CLOCK_H
#define DOSLIB_PLATFORM_BIOS_CLOCK_H

#include "../clock.h"

/* The BIOS tick counter (it starts again at midnight) and the DOS date. */
class BiosClock : public Clock {
public:
  unsigned long ms();
  unsigned ticks();
  unsigned today();
};

#endif
