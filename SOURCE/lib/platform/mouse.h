/*
   A mouse with text-mode coordinates (row, column of the cell).
*/

#ifndef DOSLIB_PLATFORM_MOUSE_H
#define DOSLIB_PLATFORM_MOUSE_H

struct MouseState {
  int row, col;
  unsigned char buttons; /* bit 0 left, bit 1 right */
  signed char wheel;     /* < 0 up, > 0 down */
};

class Mouse {
public:
  /* Looks for a driver (and its wheel); false when there is none. The other calls do
     nothing without a driver. */
  virtual bool detect() = 0;
  virtual bool present() const = 0;
  virtual bool hasWheel() const = 0;
  virtual void reset() = 0;
  virtual void show() = 0;
  virtual void hide() = 0;
  virtual void poll( MouseState &state ) = 0;
  /* Presses of the left button since the last call. */
  virtual unsigned leftPresses() = 0;
};

#endif
