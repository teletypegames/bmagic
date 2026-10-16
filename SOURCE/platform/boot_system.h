/*
   What BMAGIC needs from DOS beyond DOSLIB: the boot drive, the attributes of a file
   (MSDOS.SYS is read-only, hidden and system, and keeps that after it is saved), and the
   date and time for the backups.
*/

#ifndef BMAGIC_PLATFORM_BOOT_SYSTEM_H
#define BMAGIC_PLATFORM_BOOT_SYSTEM_H

class BootSystem {
public:
  /* The drive DOS booted from ('C'); 'C' when DOS cannot tell (before 4.0). */
  virtual char bootDrive() = 0;
  virtual bool setAttributes( const char *path, unsigned attrib ) = 0;
  /* Now, in the DOS date and time formats. */
  virtual void now( unsigned &date, unsigned &time ) = 0;
};

#endif
