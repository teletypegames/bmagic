/*
   The startup options in the [Options] block of the MSDOS.SYS of Windows 95 / 98 (a text
   file there; in MS-DOS 6 it is a binary system file, not edited):

     [Options]
     BootMenu=1          the Startup Menu of Windows at every boot (F8 shows it anyway)
     BootMenuDefault=6   its item that starts after the delay
     BootMenuDelay=10    seconds
     BootGUI=0           0: stay at the command prompt, 1: start Windows
     Logo=0              the logo while it boots
     BootKeys=1          the F5 / F8 keys at boot

   write() changes only the lines of the changed values (an empty value loses its line,
   a new one goes after the last key of the block); the other lines, among them the
   padding that keeps MSDOS.SYS over 1024 bytes, stay as they are.
*/

#ifndef BMAGIC_MODEL_STARTUP_OPTIONS_H
#define BMAGIC_MODEL_STARTUP_OPTIONS_H

class TextFile;

enum StartupField { OptBootMenu, OptMenuDefault, OptMenuDelay, OptBootGui, OptLogo, OptBootKeys, OptionCount };

class StartupOptions {
public:
  enum { MaxValue = 3 };

  /* The key in MSDOS.SYS ("BootMenu"). */
  static const char *key( int field );
  /* A 0 / 1 switch (or a number). */
  static bool isSwitch( int field );
  /* An [Options] or [Paths] block: the MSDOS.SYS of Windows. */
  static bool isWindowsFile( const TextFile &msdos );

  StartupOptions();
  void read( const TextFile &msdos );
  char *value( int field ) { return values_[field]; }
  /* The first value that is not a number (a switch: 0 or 1), or -1. */
  int invalidField() const;
  /* false when the file is full. */
  bool write( TextFile &msdos ) const;

private:
  char values_[OptionCount][MaxValue + 1];
};

#endif
