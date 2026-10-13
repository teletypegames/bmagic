#include <string.h>

#include "backup_service.h"
#include "../lib/core/str.h"
#include "../model/backup_list.h"
#include "../model/boot_docs.h"
#include "../model/limits.h"
#include "../model/text_file.h"
#include "../platform/boot_system.h"
#include "../repository/backup_repository.h"
#include "../repository/boot_file_repository.h"

static const unsigned KeptAttributes = 0x27;

BackupService::BackupService( BootFileRepository &files, BackupRepository &backups, BootSystem &system, BootDocs &docs,
                              BackupList &sets, char *buf, char drive )
  : files_( files ), backups_( backups ), system_( system ), docs_( docs ), sets_( sets ), buf_( buf ),
    drive_( drive ) {}

void BackupService::refresh() {
  backups_.list( sets_ );
}

int BackupService::backup( const char *note ) {
  refresh();
  BackupSet set;
  memset( &set, 0, sizeof( set ) );
  set.number = sets_.nextNumber();
  if ( set.number == 0 ) return 0;
  unsigned date, time;
  system_.now( date, time );
  set.setWhen( date, time );
  set.drive = drive_;
  copyStr( set.note, note, sizeof( set.note ) );
  if ( !backups_.create( set ) ) return 0;
  for ( int f = 0; f < BootFileCount; f++ ) {
    unsigned len, attrib;
    if ( files_.read( f, buf_, MaxFileSize + 1, len, attrib ) != BootFileRepository::Loaded ) continue;
    if ( TextFile::isBinary( buf_, len ) ) continue;
    if ( !backups_.writeFile( set.number, f, buf_, len ) ) {
      backups_.remove( set.number );
      return 0;
    }
  }
  refresh();
  return set.number;
}

bool BackupService::restore( int set, int &made ) {
  char note[BackupSet::MaxNote + 1] = "Before restoring ";
  char *d = note + strlen( note );
  *d++ = (char)( '0' + set / 100 % 10 );
  *d++ = (char)( '0' + set / 10 % 10 );
  *d++ = (char)( '0' + set % 10 );
  *d = 0;
  made = backup( note );
  if ( made == 0 ) return false;
  bool ok = true;
  for ( int f = 0; f < BootFileCount; f++ ) {
    unsigned len;
    if ( backups_.readFile( set, f, buf_, MaxFileSize + 1, len ) != BackupRepository::Loaded ) continue;
    if ( !files_.write( f, buf_, len ) ) {
      ok = false;
      continue;
    }
    unsigned attrib = docs_.attrib[f] & KeptAttributes;
    if ( attrib & 0x07 ) {
      char path[PathSize];
      files_.path( f, path );
      system_.setAttributes( path, attrib );
    }
  }
  return ok;
}

bool BackupService::remove( int set ) {
  bool ok = backups_.remove( set );
  refresh();
  return ok;
}

bool BackupService::load( int set, int file, TextFile &text ) {
  unsigned len;
  BackupRepository::Read read = backups_.readFile( set, file, buf_, MaxFileSize + 1, len );
  text.clear();
  if ( read == BackupRepository::Missing ) return true;
  return read == BackupRepository::Loaded && !TextFile::isBinary( buf_, len ) && text.load( buf_, len );
}
