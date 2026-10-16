/*
   The keys BMAGIC uses beyond lib/ui/keys.h. Ctrl+Up / Ctrl+Down come only from the
   enhanced keyboard functions of the BIOS, which DOSLIB does not use: moving is
   Ctrl+PgUp / Ctrl+PgDn, and the Ctrl+arrows work where they arrive.
*/

#ifndef BMAGIC_CONTROLLER_KEY_CODES_H
#define BMAGIC_CONTROLLER_KEY_CODES_H

enum { ScanIns = 0x52, ScanCtrlUp = 0x8D, ScanCtrlDown = 0x91, ScanCtrlLeft = 0x73, ScanCtrlRight = 0x74 };

#endif
