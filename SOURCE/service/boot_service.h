/*
   Loading and saving the boot files. Saving first backs up the files as they are on the
   disk (BackupService), then writes the changed ones and gives them back their
   read-only, hidden and system attributes.
*/

#ifndef BMAGIC_SERVICE_BOOT_SERVICE_H
#define BMAGIC_SERVICE_BOOT_SERVICE_H

class BackupService;
class BootFileRepository;
class BootSystem;
struct BootDocs;

class BootService {
public:
  enum Save { Saved, Unchanged, BackupFailed, WriteFailed };

  /* buf: TextFile::OutSize bytes */
  BootService( BootFileRepository &files, BackupService &backups, BootSystem &system, BootDocs &docs, char *buf );
  /* Both versions of each file from the disk. */
  void load();
  bool isChanged( int file ) const;
  bool isChanged() const;
  /* The edits are dropped. */
  void discard();
  /* set: the number of the backup; failed: the file that could not be written. */
  Save save( int &set, int &failed );

private:
  void loadFile( int file );
  bool writeFile( int file );

  BootFileRepository &files_;
  BackupService &backups_;
  BootSystem &system_;
  BootDocs &docs_;
  char *buf_;
};

#endif
