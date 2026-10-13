/*
   The backups of the boot files: a new set of the files as they are on the disk (byte for
   byte), the list of the sets, a file of a set for a diff, restoring a set (after a
   backup of the files it replaces), deleting a set.
*/

#ifndef BMAGIC_SERVICE_BACKUP_SERVICE_H
#define BMAGIC_SERVICE_BACKUP_SERVICE_H

class BackupList;
class BackupRepository;
class BootFileRepository;
class BootSystem;
class TextFile;
struct BootDocs;

class BackupService {
public:
  /* buf: TextFile::OutSize bytes */
  BackupService( BootFileRepository &files, BackupRepository &backups, BootSystem &system, BootDocs &docs,
                 BackupList &sets, char *buf, char drive );
  /* The sets into the list. */
  void refresh();
  /* A new set of the text files on the disk: its number, 0 when it could not be made. */
  int backup( const char *note );
  /* The files of the set on the disk; made: the backup made before (0: none, failed). The
     caller loads the files again. */
  bool restore( int set, int &made );
  bool remove( int set );
  /* A file of a set (empty when the set does not have it); false when it is too big. */
  bool load( int set, int file, TextFile &text );

private:
  BootFileRepository &files_;
  BackupRepository &backups_;
  BootSystem &system_;
  BootDocs &docs_;
  BackupList &sets_;
  char *buf_;
  char drive_;
};

#endif
