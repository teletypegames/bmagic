#include <dos.h>

#include "dos_speaker.h"

void DosSpeaker::beep() {
  bdos( 2, 7, 0 );
}
