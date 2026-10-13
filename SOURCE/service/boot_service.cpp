#include "backup_service.h"
#include "boot_service.h"
#include "../model/boot_docs.h"
#include "../model/limits.h"
#include "../model/text_file.h"
#include "../platform/boot_system.h"
#include "../repository/boot_file_repository.h"

static const unsigned KeptAttributes = 0x27; /* read-only, hidden, system, archive */

BootService::BootService( BootFileRepository &files, BackupService &backups, BootSystem &system, BootDocs &docs,
                          char *buf )
  : files_( files ), backups_( backups ), system_( system ), docs_( docs ), buf_( buf ) {}

void BootService::loadFile( int file ) {
  unsigned len, attrib;
  TextFile &disk = *docs_.disk[file];
  BootFileRepository::Read read = files_.read( file, buf_, MaxFileSize + 1, len, attrib );
  docs_.attrib[file] = attrib;
  disk.clear();
  if ( read == BootFileRepository::Missing ) docs_.state[file] = FileMissing;
  else if ( read == BootFileRepository::TooBig ) docs_.state[file] = FileTooBig;
  else if ( read == BootFileRepository::Unreadable ) docs_.state[file] = FileUnreadable;
  else if ( TextFile::isBinary( buf_, len ) ) docs_.state[file] = FileBinary;
  else docs_.state[file] = disk.load( buf_, len ) ? FileText : FileTooBig;
  if ( docs_.state[file] != FileText ) disk.clear();
  *docs_.edit[file] = disk;
}

void BootService::load() {
  for ( int f = 0; f < BootFileCount; f++ ) loadFile( f );
}

bool BootService::isChanged( int file ) const {
  return docs_.isEditable( file ) && !docs_.edit[file]->sameAs( *docs_.disk[file] );
}

bool BootService::isChanged() const {
  for ( int f = 0; f < BootFileCount; f++ ) {
    if ( isChanged( f ) ) return true;
  }
  return false;
}

void BootService::discard() {
  for ( int f = 0; f < BootFileCount; f++ ) *docs_.edit[f] = *docs_.disk[f];
}

bool BootService::writeFile( int file ) {
  unsigned len = docs_.edit[file]->write( buf_ );
  if ( !files_.write( file, buf_, len ) ) return false;
  unsigned attrib = docs_.attrib[file] & KeptAttributes;
  if ( attrib & 0x07 ) {
    char path[PathSize];
    files_.path( file, path );
    system_.setAttributes( path, attrib );
  }
  *docs_.disk[file] = *docs_.edit[file];
  docs_.state[file] = FileText;
  return true;
}

BootService::Save BootService::save( int &set, int &failed ) {
  set = 0;
  failed = -1;
  if ( !isChanged() ) return Unchanged;
  set = backups_.backup( "Before saving" );
  if ( set == 0 ) return BackupFailed;
  for ( int f = 0; f < BootFileCount; f++ ) {
    if ( isChanged( f ) && !writeFile( f ) ) {
      failed = f;
      return WriteFailed;
    }
  }
  return Saved;
}
