#ifndef DOSLIB_PLATFORM_SPEAKER_H
#define DOSLIB_PLATFORM_SPEAKER_H

/* The beep that says "that does not work here". */
class Speaker {
public:
  virtual void beep() = 0;
};

#endif
