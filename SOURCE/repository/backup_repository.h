/*
   The backup sets (see BackupList), each a folder of copies of the boot files.
*/

#ifndef BMAGIC_REPOSITORY_BACKUP_REPOSITORY_H
#define BMAGIC_REPOSITORY_BACKUP_REPOSITORY_H

class BackupList;
struct BackupSet;

class BackupRepository {
public:
  enum Read { Missing, Loaded, TooBig };

  virtual void list( BackupList &sets ) = 0;
  /* A new empty set with set.number, with its SET.INI; false when it cannot be made. */
  virtual bool create( const BackupSet &set ) = 0;
  virtual bool writeFile( int set, int file, const char *text, unsigned len ) = 0;
  /* The copy into buf (at most size - 1 bytes, terminated), its length in len. */
  virtual Read readFile( int set, int file, char *buf, unsigned size, unsigned &len ) = 0;
  /* The set with its files. */
  virtual bool remove( int set ) = 0;
  /* "C:\APPS\BMAGIC\BACKUP\003" */
  virtual void folder( int set, char *out ) const = 0;
};

#endif
