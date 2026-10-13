#ifndef DOSLIB_PLATFORM_CLOCK_H
#define DOSLIB_PLATFORM_CLOCK_H

class Clock {
public:
  /* Milliseconds from an arbitrary start. */
  virtual unsigned long ms() = 0;
  /* Timer ticks (18.2 per second) from an arbitrary start. */
  virtual unsigned ticks() = 0;
  /* Today in the DOS date format: (year - 1980) << 9 | month << 5 | day. */
  virtual unsigned today() = 0;
};

#endif
