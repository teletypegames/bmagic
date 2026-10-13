/*
   Test builds: the keys come from a text of hex BIOS key codes ("5000 1C0D"). When they
   run out (or at a 0), the ScriptEnd decides what happens - typically the screen is
   saved and the program ends or gets an Esc.
*/

#ifndef DOSLIB_TESTING_SCRIPTED_KEYBOARD_H
#define DOSLIB_TESTING_SCRIPTED_KEYBOARD_H

#include "../platform/keyboard.h"

class ScriptEnd {
public:
  /* The key the program gets instead (if it returns at all). */
  virtual unsigned scriptEnded() = 0;
};

class ScriptedKeyboard : public Keyboard {
public:
  ScriptedKeyboard( const char *keys, ScriptEnd &end );
  void begin();
  void end();
  bool ready();
  unsigned read();
  bool shiftDown();

private:
  const char *next_;
  ScriptEnd &end_;
};

#endif
