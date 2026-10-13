#include <string.h>

#include "folder_backup_repository.h"
#include "../../lib/core/str.h"
#include "../../lib/platform/file_system.h"
#include "../../model/backup_list.h"
#include "../../model/boot_file.h"
#include "../../model/limits.h"

FolderBackupRepository::FolderBackupRepository( FileSystem &fs, const char *progDir )
  : fs_( fs ), progDir_( progDir ) {}

void FolderBackupRepository::folder( int set, char *out ) const {
  strcpy( out, progDir_ );
  strcat( out, "BACKUP" );
  if ( set <= 0 ) return;
  char *d = out + strlen( out );
  *d++ = '\\';
  *d++ = (char)( '0' + set / 100 % 10 );
  *d++ = (char)( '0' + set / 10 % 10 );
  *d++ = (char)( '0' + set % 10 );
  *d = 0;
}

void FolderBackupRepository::file( int set, const char *name, char *out ) const {
  folder( set, out );
  joinLocal( out, out, name );
}

/* "001".."999": its number, otherwise 0. */
static int setNumber( const char *name ) {
  if ( strlen( name ) != 3 ) return 0;
  int n = 0;
  for ( int i = 0; i < 3; i++ ) {
    if ( name[i] < '0' || name[i] > '9' ) return 0;
    n = n * 10 + name[i] - '0';
  }
  return n;
}

class SetFolders : public DirVisitor {
public:
  explicit SetFolders( BackupList &sets ) : sets_( sets ) {}
  bool visit( const DirEntry &entry ) {
    BackupSet set;
    memset( &set, 0, sizeof( set ) );
    set.number = setNumber( entry.name );
    if ( entry.isFolder() && set.number ) sets_.add( set );
    return true;
  }

private:
  BackupList &sets_;
};

void FolderBackupRepository::list( BackupList &sets ) {
  char path[PathSize];
  folder( 0, path );
  strcat( path, "\\*.*" );
  sets.clear();
  SetFolders folders( sets );
  fs_.list( path, AttrFolder, folders );
  for ( int i = 0; i < sets.count(); i++ ) {
    BackupSet &set = sets.at( i );
    char info[256];
    file( set.number, "SET.INI", path );
    if ( readText( fs_, path, info, sizeof( info ) ) >= 0 ) set.readInfo( info );
    for ( int f = 0; f < BootFileCount; f++ ) {
      file( set.number, bootFileName( f ), path );
      if ( fs_.exists( path ) ) set.files |= (unsigned char)( 1 << f );
    }
  }
}

bool FolderBackupRepository::create( const BackupSet &set ) {
  char path[PathSize], info[160];
  folder( 0, path );
  makeFolders( fs_, path );
  folder( set.number, path );
  if ( !fs_.makeFolder( path ) ) return false;
  file( set.number, "SET.INI", path );
  return ::writeFile( fs_, path, info, set.writeInfo( info ) );
}

bool FolderBackupRepository::writeFile( int set, int f, const char *text, unsigned len ) {
  char path[PathSize];
  file( set, bootFileName( f ), path );
  return ::writeFile( fs_, path, text, len );
}

BackupRepository::Read FolderBackupRepository::readFile( int set, int f, char *buf, unsigned size, unsigned &len ) {
  char path[PathSize];
  file( set, bootFileName( f ), path );
  len = 0;
  int n = readText( fs_, path, buf, size );
  if ( n < 0 ) return Missing;
  if ( n >= (int)size - 1 ) return TooBig;
  len = n;
  return Loaded;
}

class FolderFiles : public DirVisitor {
public:
  FolderFiles( FileSystem &fs, const char *folder ) : ok( true ), fs_( fs ), folder_( folder ) {}
  bool visit( const DirEntry &entry ) {
    if ( entry.isFolder() ) return true;
    char path[PathSize];
    joinLocal( path, folder_, entry.name );
    fs_.makeWritable( path );
    ok = fs_.remove( path ) && ok;
    return true;
  }
  bool ok;

private:
  FileSystem &fs_;
  const char *folder_;
};

bool FolderBackupRepository::remove( int set ) {
  char path[PathSize], pattern[PathSize];
  folder( set, path );
  joinLocal( pattern, path, "*.*" );
  FolderFiles files( fs_, path );
  fs_.list( pattern, AttrReadOnly | AttrHidden | AttrSystem, files );
  return fs_.removeFolder( path ) && files.ok;
}
