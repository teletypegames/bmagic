#ifndef DOSLIB_PLATFORM_DOS_IDLE_H
#define DOSLIB_PLATFORM_DOS_IDLE_H

#include "../idle.h"

/* Gives the time slice to a multitasker (Windows, OS/2, DOSBox): INT 2Fh 1680h. */
class DosIdle : public Idle {
public:
  void idle();
};

#endif
