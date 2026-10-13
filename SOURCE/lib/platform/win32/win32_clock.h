#ifndef DOSLIB_PLATFORM_WIN32_CLOCK_H
#define DOSLIB_PLATFORM_WIN32_CLOCK_H

#include "../clock.h"

class Win32Clock : public Clock {
public:
  unsigned long ms();
  unsigned ticks();
  unsigned today();
};

#endif
