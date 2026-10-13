#ifndef DOSLIB_PLATFORM_DOS_SPEAKER_H
#define DOSLIB_PLATFORM_DOS_SPEAKER_H

#include "../speaker.h"

/* A BEL character through DOS. */
class DosSpeaker : public Speaker {
public:
  void beep();
};

#endif
