#ifndef DOSLIB_PLATFORM_INT33_MOUSE_H
#define DOSLIB_PLATFORM_INT33_MOUSE_H

#include "../mouse.h"

/* The mouse driver on INT 33h; the wheel through the CuteMouse WheelAPI. */
class Int33Mouse : public Mouse {
public:
  Int33Mouse();
  bool detect();
  bool present() const;
  bool hasWheel() const;
  void reset();
  void show();
  void hide();
  void poll( MouseState &state );
  unsigned leftPresses();

private:
  bool present_;
  bool wheel_;
};

#endif
