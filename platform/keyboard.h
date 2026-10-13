/*
   The keyboard as BIOS key codes: scan code << 8 | ASCII.
*/

#ifndef DOSLIB_PLATFORM_KEYBOARD_H
#define DOSLIB_PLATFORM_KEYBOARD_H

class Keyboard {
public:
  /* Raw key input on and off (a console otherwise edits lines and stops on Ctrl+C). */
  virtual void begin() = 0;
  virtual void end() = 0;
  virtual bool ready() = 0;
  /* Waits for a key. */
  virtual unsigned read() = 0;
  virtual bool shiftDown() = 0;
};

#endif
