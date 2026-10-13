#include <string.h>

#include "drive_boot_file_repository.h"
#include "../../lib/platform/file_system.h"
#include "../../model/boot_file.h"

DriveBootFileRepository::DriveBootFileRepository( FileSystem &fs, char drive ) : fs_( fs ), drive_( drive ) {}

void DriveBootFileRepository::path( int file, char *out ) const {
  out[0] = drive_;
  strcpy( out + 1, ":\\" );
  strcat( out, bootFileName( file ) );
}

void DriveBootFileRepository::pathWith( int file, const char *ext, char *out ) const {
  path( file, out );
  strcpy( strrchr( out, '.' ), ext );
}

BootFileRepository::Read DriveBootFileRepository::read( int file, char *buf, unsigned size, unsigned &len,
                                                        unsigned &attrib ) {
  char name[20];
  path( file, name );
  DirEntry entry;
  len = 0;
  attrib = 0;
  buf[0] = 0;
  if ( !fs_.find( name, AttrReadOnly | AttrHidden | AttrSystem, entry ) ) return Missing;
  attrib = entry.attrib;
  int n = readText( fs_, name, buf, size );
  if ( n < 0 ) return Unreadable;
  if ( n >= (int)size - 1 ) return TooBig;
  len = n;
  return Loaded;
}

bool DriveBootFileRepository::write( int file, const char *text, unsigned len ) {
  char name[20], temp[20], old[20];
  path( file, name );
  pathWith( file, ".$$$", temp );
  pathWith( file, ".BM$", old );
  if ( !writeFile( fs_, temp, text, len ) ) {
    fs_.remove( temp );
    return false;
  }
  DirEntry entry;
  bool existed = fs_.find( name, AttrReadOnly | AttrHidden | AttrSystem, entry );
  if ( existed ) {
    fs_.makeWritable( name );
    fs_.remove( old );
    if ( !fs_.rename( name, old ) ) {
      fs_.remove( temp );
      return false;
    }
  }
  if ( !fs_.rename( temp, name ) ) {
    if ( existed ) fs_.rename( old, name );
    fs_.remove( temp );
    return false;
  }
  if ( existed ) fs_.remove( old );
  return true;
}
