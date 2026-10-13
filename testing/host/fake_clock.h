#ifndef DOSLIB_TESTING_FAKE_CLOCK_H
#define DOSLIB_TESTING_FAKE_CLOCK_H

#include "../../platform/clock.h"

/* A clock that moves only when the test moves it. */
class FakeClock : public Clock {
public:
  FakeClock() : now( 0 ), date( ( ( 2026 - 1980 ) << 9 ) | ( 10 << 5 ) | 11 ) {}
  unsigned long ms() { return now; }
  unsigned ticks() { return (unsigned)( now / 55 ); }
  unsigned today() { return date; }

  unsigned long now;
  unsigned date;
};

#endif
