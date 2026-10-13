#ifndef BMAGIC_REPOSITORY_DRIVE_BOOT_FILE_REPOSITORY_H
#define BMAGIC_REPOSITORY_DRIVE_BOOT_FILE_REPOSITORY_H

#include "../boot_file_repository.h"

class FileSystem;

/* The boot files in X:\. A file is replaced in three steps, so that it is never lost:
   X.$$$ is written, the old file is renamed to X.BM$, X.$$$ to the file, then X.BM$ is
   deleted (on a failure the old file is put back). */
class DriveBootFileRepository : public BootFileRepository {
public:
  DriveBootFileRepository( FileSystem &fs, char drive );
  Read read( int file, char *buf, unsigned size, unsigned &len, unsigned &attrib );
  bool write( int file, const char *text, unsigned len );
  void path( int file, char *out ) const;

private:
  /* The file's path with another extension (".$$$"). */
  void pathWith( int file, const char *ext, char *out ) const;

  FileSystem &fs_;
  char drive_;
};

#endif
