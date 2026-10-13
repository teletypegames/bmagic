#include <stddef.h>
#include <string.h>

#include "diff_controller.h"
#include "../lib/core/str.h"
#include "../lib/platform/keyboard.h"
#include "../lib/ui/keys.h"
#include "../model/boot_docs.h"
#include "../model/text_diff.h"
#include "../service/backup_service.h"
#include "../view/diff_view.h"
#include "../view/frame.h"

enum { ShiftStep = 8 };

DiffController::DiffController( DiffView &view, Frame &frame, Keyboard &keyboard, TextDiff &diff, BootDocs &docs,
                                BackupService &backups, TextFile &backup )
  : view_( view ), frame_( frame ), keyboard_( keyboard ), diff_( diff ), docs_( docs ), backups_( backups ),
    backup_( backup ), set_( 0 ) {}

bool DiffController::pair( int file, const TextFile *&before, const TextFile *&after ) {
  if ( set_ == 0 ) {
    if ( !docs_.isEditable( file ) ) return false;
    before = docs_.disk[file];
    after = docs_.edit[file];
    return true;
  }
  if ( docs_.state[file] != FileText && docs_.state[file] != FileMissing ) return false;
  if ( !backups_.load( set_, file, backup_ ) ) return false;
  before = &backup_;
  after = docs_.disk[file];
  return true;
}

DiffController::Result DiffController::showEdits( bool confirm ) {
  set_ = 0;
  return run( confirm ? "Review the changes, then F2 saves them" : "Changes not saved yet", confirm );
}

DiffController::Result DiffController::showBackup( int set ) {
  char title[60] = "Backup ";
  char *d = title + strlen( title );
  *d++ = (char)( '0' + set / 100 % 10 );
  *d++ = (char)( '0' + set / 10 % 10 );
  *d++ = (char)( '0' + set % 10 );
  strcpy( d, " -> the files now on the disk" );
  set_ = set;
  return run( title, false );
}

DiffController::Result DiffController::run( const char *title, bool confirm ) {
  int files[BootFileCount], count = 0;
  const TextFile *before, *after;
  for ( int f = 0; f < BootFileCount; f++ ) {
    if ( !pair( f, before, after ) ) continue;
    diff_.compare( *before, *after );
    if ( !diff_.isSame() ) files[count++] = f;
  }
  if ( count == 0 ) return NoChanges;
  int k = 0, top = 0, shift = 0;
  bool load = true;
  for ( ;; ) {
    if ( load ) {
      pair( files[k], before, after );
      diff_.compare( *before, *after );
      top = shift = 0;
      load = false;
    }
    char part[30];
    strcpy( part, bootFileName( files[k] ) );
    if ( count > 1 ) {
      char *d = part + strlen( part );
      *d++ = ' ';
      *d++ = (char)( '1' + k );
      *d++ = '/';
      *d++ = (char)( '0' + count );
      *d = 0;
    }
    view_.draw( title, part, diff_, *before, *after, top, shift, confirm );
    frame_.message( NULL );
    frame_.flush();
    unsigned key = keyboard_.read();
    unsigned char ch = keyAscii( key ), scan = keyScan( key );
    int last = diff_.count() > DiffView::Rows ? diff_.count() - DiffView::Rows : 0;
    if ( ch == KeyEsc ) return Back;
    if ( ch == KeyTab ) {
      k = ( k + 1 ) % count;
      load = true;
      continue;
    }
    if ( !isExtendedKey( key ) ) continue;
    switch ( scan ) {
      case ScanF2:
        if ( confirm ) return Confirmed;
        break;
      case ScanUp: top--; break;
      case ScanDown: top++; break;
      case ScanPgUp: top -= DiffView::Rows - 1; break;
      case ScanPgDn: top += DiffView::Rows - 1; break;
      case ScanHome: top = 0; break;
      case ScanEnd: top = last; break;
      case ScanLeft: shift = shift > ShiftStep ? shift - ShiftStep : 0; break;
      case ScanRight:
        if ( shift < 200 ) shift += ShiftStep;
        break;
    }
    if ( top > last ) top = last;
    if ( top < 0 ) top = 0;
  }
}
