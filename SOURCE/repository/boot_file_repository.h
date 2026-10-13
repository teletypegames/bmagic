/*
   The boot files in the root folder of a drive.
*/

#ifndef BMAGIC_REPOSITORY_BOOT_FILE_REPOSITORY_H
#define BMAGIC_REPOSITORY_BOOT_FILE_REPOSITORY_H

class BootFileRepository {
public:
  enum Read { Missing, Loaded, TooBig, Unreadable };

  /* The file into buf (at most size - 1 bytes, terminated): its length and attributes. */
  virtual Read read( int file, char *buf, unsigned size, unsigned &len, unsigned &attrib ) = 0;
  /* Writes X.$$$ next to it, then puts it in the file's place (clearing the read-only,
     hidden and system attributes of the old one); false when it could not be written. */
  virtual bool write( int file, const char *text, unsigned len ) = 0;
  /* "C:\CONFIG.SYS" */
  virtual void path( int file, char *out ) const = 0;
};

#endif
