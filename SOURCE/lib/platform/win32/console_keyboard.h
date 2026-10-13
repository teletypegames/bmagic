#ifndef DOSLIB_PLATFORM_CONSOLE_KEYBOARD_H
#define DOSLIB_PLATFORM_CONSOLE_KEYBOARD_H

#include <windows.h>

#include "../keyboard.h"

/* Console key events as BIOS key codes; the shift keys alone are not keys. */
class ConsoleKeyboard : public Keyboard {
public:
  ConsoleKeyboard();
  void begin();
  void end();
  bool ready();
  unsigned read();
  bool shiftDown();

private:
  HANDLE input_;
  DWORD oldMode_;
  unsigned pending_;
  bool shift_;
};

#endif
