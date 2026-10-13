/*
   The backups of the boot files. A backup is a numbered set (001-999) of copies of the
   files as they were on the disk; its SET.INI tells when and why it was made:

     [backup]
     date=2026-10-13
     time=14:05
     drive=C
     note=Before saving
*/

#ifndef BMAGIC_MODEL_BACKUP_LIST_H
#define BMAGIC_MODEL_BACKUP_LIST_H

struct BackupSet {
  enum { MaxNote = 30 };

  int number;
  char date[11]; /* YYYY-MM-DD */
  char time[6];  /* HH:MM */
  char drive;
  char note[MaxNote + 1];
  unsigned char files; /* bit 1 << BootFile: the file is in the set */

  /* date and time from the DOS formats. */
  void setWhen( unsigned dosDate, unsigned dosTime );
  /* SET.INI into out (160 bytes): its length. */
  unsigned writeInfo( char *out ) const;
  void readInfo( const char *text );
  bool has( int file ) const { return ( files >> file ) & 1; }
};

class BackupList {
public:
  enum { Capacity = 100, MaxNumber = 999 };

  BackupList();
  void clear();
  int count() const { return count_; }
  /* Newest (highest number) first. */
  BackupSet &at( int i ) { return sets_[i]; }
  /* In its place; false when the list is full (the oldest are left out). */
  bool add( const BackupSet &set );
  /* The number of the next set, 0 when 999 is taken. */
  int nextNumber() const;

private:
  BackupSet sets_[Capacity];
  int count_;
  int highest_;
};

#endif
