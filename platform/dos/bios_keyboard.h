#ifndef DOSLIB_PLATFORM_BIOS_KEYBOARD_H
#define DOSLIB_PLATFORM_BIOS_KEYBOARD_H

#include "../keyboard.h"

class BiosKeyboard : public Keyboard {
public:
  void begin();
  void end();
  bool ready();
  unsigned read();
  bool shiftDown();
};

#endif
