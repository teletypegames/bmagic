#ifndef DOSLIB_PLATFORM_WIN32_IDLE_H
#define DOSLIB_PLATFORM_WIN32_IDLE_H

#include "../idle.h"

class Win32Idle : public Idle {
public:
  void idle();
};

#endif
