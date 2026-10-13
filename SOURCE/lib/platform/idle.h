#ifndef DOSLIB_PLATFORM_IDLE_H
#define DOSLIB_PLATFORM_IDLE_H

/* What a program does while it waits for input: give the time slice away, or keep a
   network stack running. */
class Idle {
public:
  virtual void idle() = 0;
};

#endif
