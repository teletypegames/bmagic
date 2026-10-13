#ifndef BMAGIC_REPOSITORY_FOLDER_BACKUP_REPOSITORY_H
#define BMAGIC_REPOSITORY_FOLDER_BACKUP_REPOSITORY_H

#include "../backup_repository.h"

class FileSystem;

/* The sets in <program folder>\BACKUP\001, \002, ...: the copies of the boot files under
   their own names, and SET.INI. */
class FolderBackupRepository : public BackupRepository {
public:
  /* progDir: with a trailing '\' */
  FolderBackupRepository( FileSystem &fs, const char *progDir );
  void list( BackupList &sets );
  bool create( const BackupSet &set );
  bool writeFile( int set, int file, const char *text, unsigned len );
  Read readFile( int set, int file, char *buf, unsigned size, unsigned &len );
  bool remove( int set );
  void folder( int set, char *out ) const;

private:
  void file( int set, const char *name, char *out ) const;

  FileSystem &fs_;
  const char *progDir_;
};

#endif
