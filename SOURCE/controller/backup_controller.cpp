#include <stddef.h>
#include <string.h>

#include "backup_controller.h"
#include "diff_controller.h"
#include "key_codes.h"
#include "list_keys.h"
#include "../lib/core/str.h"
#include "../lib/platform/keyboard.h"
#include "../lib/ui/keys.h"
#include "../model/backup_list.h"
#include "../service/backup_service.h"
#include "../service/boot_service.h"
#include "../view/backup_view.h"
#include "../view/frame.h"

BackupController::BackupController( BackupView &view, Frame &frame, Keyboard &keyboard, BackupService &backups,
                                    BootService &boot, BackupList &sets, DiffController &diffs, const char *folder )
  : view_( view ), frame_( frame ), keyboard_( keyboard ), backups_( backups ), boot_( boot ), sets_( sets ),
    diffs_( diffs ), folder_( folder ), message_( NULL ), error_( false ) {
  cursor_.setRows( BackupView::Rows );
}

/* text + " 003" + rest into text_. */
static void withNumber( char *out, const char *text, int set, const char *rest ) {
  strcpy( out, text );
  char *d = out + strlen( out );
  *d++ = (char)( '0' + set / 100 % 10 );
  *d++ = (char)( '0' + set / 10 % 10 );
  *d++ = (char)( '0' + set % 10 );
  strcpy( d, rest );
}

bool BackupController::ask( const char *question, int set ) {
  withNumber( text_, question, set, "? (Y/N)" );
  frame_.error( text_ );
  frame_.flush();
  unsigned char answer = keyAscii( keyboard_.read() );
  return answer == 'y' || answer == 'Y';
}

void BackupController::compare() {
  if ( diffs_.showBackup( sets_.at( cursor_.sel() ).number ) == DiffController::NoChanges )
    message_ = "The files on the disk are the same as in this backup.";
}

void BackupController::backupNow() {
  int set = backups_.backup( "Made by hand" );
  error_ = set == 0;
  if ( set == 0 ) message_ = "The backup could not be made (the disk is full or write-protected?)";
  else {
    withNumber( text_, "Backup ", set, " made." );
    message_ = text_;
    cursor_.place( 0, 0 );
  }
}

bool BackupController::restore() {
  int set = sets_.at( cursor_.sel() ).number;
  const char *question = boot_.isChanged() ? "The edits not saved yet are dropped. Restore backup "
                                           : "The files on the disk are backed up, then replaced. Restore backup ";
  if ( !ask( question, set ) ) return false;
  int made;
  bool ok = backups_.restore( set, made );
  boot_.load();
  backups_.refresh();
  error_ = !ok;
  if ( made == 0 ) message_ = "Nothing was restored: the backup before it could not be made.";
  else if ( !ok ) message_ = "A file could not be written (the disk is full or write-protected?)";
  else {
    withNumber( text_, "Restored. The files it replaced are in backup ", made, "." );
    message_ = text_;
  }
  cursor_.place( 0, 0 );
  return made != 0;
}

void BackupController::remove() {
  int set = sets_.at( cursor_.sel() ).number;
  if ( !ask( "Delete backup ", set ) ) return;
  error_ = !backups_.remove( set );
  message_ = error_ ? "The backup could not be deleted." : "Deleted.";
}

bool BackupController::run() {
  bool restored = false;
  backups_.refresh();
  cursor_.place( 0, 0 );
  message_ = NULL;
  error_ = false;
  for ( ;; ) {
    cursor_.setCount( sets_.count() );
    cursor_.clamp();
    view_.draw( sets_, cursor_, folder_ );
    if ( error_ ) frame_.error( message_ );
    else frame_.message( message_ );
    frame_.flush();
    message_ = NULL;
    error_ = false;
    unsigned key = keyboard_.read();
    unsigned char ch = keyAscii( key );
    bool any = sets_.count() > 0;
    if ( ch == KeyEsc ) return restored;
    if ( moveInList( cursor_, key ) ) continue;
    if ( ch == KeyEnter && any ) compare();
    else if ( isExtendedKey( key ) && keyScan( key ) == ScanIns ) backupNow();
    else if ( ( ch == 'r' || ch == 'R' ) && any ) restored = restore() || restored;
    else if ( isExtendedKey( key ) && keyScan( key ) == ScanDel && any ) remove();
  }
}
