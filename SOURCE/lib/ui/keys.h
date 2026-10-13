/*
   BIOS key codes: scan code << 8 | ASCII. The ASCII byte of a special key (arrows,
   function keys) is 0, or E0h for a gray key of the enhanced keyboard.
*/

#ifndef DOSLIB_UI_KEYS_H
#define DOSLIB_UI_KEYS_H

enum {
  KeyCtrlE = 0x05,
  KeyBackspace = 0x08,
  KeyTab = 0x09,
  KeyCtrlEnter = 0x0A,
  KeyEnter = 0x0D,
  KeyCtrlS = 0x13,
  KeyCtrlY = 0x19,
  KeyEsc = 0x1B,
  KeyDelChar = 0x7F
};

enum {
  ScanTab = 0x0F,
  ScanF1 = 0x3B,
  ScanF2 = 0x3C,
  ScanF3 = 0x3D,
  ScanF4 = 0x3E,
  ScanF5 = 0x3F,
  ScanF6 = 0x40,
  ScanF7 = 0x41,
  ScanF8 = 0x42,
  ScanF9 = 0x43,
  ScanF10 = 0x44,
  ScanHome = 0x47,
  ScanUp = 0x48,
  ScanPgUp = 0x49,
  ScanLeft = 0x4B,
  ScanRight = 0x4D,
  ScanEnd = 0x4F,
  ScanDown = 0x50,
  ScanPgDn = 0x51,
  ScanDel = 0x53,
  ScanShiftF7 = 0x5A,
  ScanCtrlPgDn = 0x76,
  ScanCtrlPgUp = 0x84
};

/* The full code of Esc, for what returns a key. */
enum { KeyEscCode = 0x011B };

inline unsigned char keyAscii( unsigned key ) {
  return (unsigned char)key;
}

inline unsigned char keyScan( unsigned key ) {
  return (unsigned char)( key >> 8 );
}

inline bool isExtendedKey( unsigned key ) {
  return keyAscii( key ) == 0 || keyAscii( key ) == 0xE0;
}

#endif
