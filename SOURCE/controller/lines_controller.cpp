#include <stddef.h>
#include <string.h>

#include "diff_controller.h"
#include "help_controller.h"
#include "key_codes.h"
#include "line_input.h"
#include "lines_controller.h"
#include "list_keys.h"
#include "../lib/core/str.h"
#include "../lib/platform/keyboard.h"
#include "../lib/platform/speaker.h"
#include "../lib/ui/keys.h"
#include "../model/boot_docs.h"
#include "../model/boot_menu.h"
#include "../model/line_rows.h"
#include "../model/menu_editor.h"
#include "../model/text_file.h"
#include "../service/boot_service.h"
#include "../view/frame.h"
#include "../view/lines_view.h"

static const char FileFull[] = "The file is full (500 lines, 16 KB)";

LinesController::LinesController( LinesView &view, Frame &frame, Keyboard &keyboard, Speaker &speaker, LineInput &input,
                                  BootDocs &docs, BootMenu &menu, MenuEditor &editor, LineRows &rows, BootService &boot,
                                  DiffController &diffs, HelpController &help, char drive )
  : view_( view ), frame_( frame ), keyboard_( keyboard ), speaker_( speaker ), input_( input ), docs_( docs ),
    menu_( menu ), editor_( editor ), rows_( rows ), boot_( boot ), diffs_( diffs ), help_( help ), drive_( drive ),
    file_( -1 ), message_( NULL ), error_( false ) {
  name_[0] = 0;
  cursor_.setRows( LinesView::Rows );
}

void LinesController::runItem( int item ) {
  file_ = -1;
  strcpy( name_, menu_.items[item].name );
  strcpy( title_, "Item " );
  strcat( title_, name_ );
  if ( menu_.items[item].text[0] ) {
    strcat( title_, ": " );
    strncat( title_, menu_.items[item].text, 40 );
  }
  run();
}

void LinesController::runFile( int file ) {
  file_ = file;
  run();
}

/* The rows from the files as they are now; false when the item is gone. */
bool LinesController::rebuild() {
  if ( file_ < 0 ) {
    int item = menu_.find( name_ );
    if ( item < 0 ) return false;
    rows_.forItem( menu_, item, *docs_.edit[ConfigSys], *docs_.edit[AutoexecBat] );
  }
  else {
    rows_.forFile( file_, *docs_.edit[file_] );
    title_[0] = drive_;
    strcpy( title_ + 1, ":\\" );
    strcat( title_, bootFileName( file_ ) );
  }
  cursor_.setCount( rows_.count() );
  cursor_.clamp();
  return true;
}

void LinesController::draw() {
  view_.draw( title_, file_ < 0 ? name_ : NULL, rows_, cursor_, docs_, drive_, boot_.isChanged() );
}

void LinesController::selectLine( int file, int line ) {
  int row = rows_.find( file, line );
  if ( row >= 0 ) cursor_.select( row );
}

void LinesController::run() {
  cursor_.place( 0, 0 );
  message_ = NULL;
  error_ = false;
  if ( !rebuild() ) return;
  for ( ;; ) {
    draw();
    if ( error_ ) frame_.error( message_ );
    else frame_.message( message_ );
    frame_.flush();
    message_ = NULL;
    error_ = false;
    if ( !handleKey( keyboard_.read() ) || !rebuild() ) return;
  }
}

bool LinesController::edit( int file, int line, bool added ) {
  TextFile &text = *docs_.edit[file];
  if ( strlen( text.line( line ) ) > MaxLine ) {
    message_ = "The line is longer than 200 characters: edit it with a text editor.";
    error_ = true;
    return false;
  }
  char value[MaxLine + 1], label[16] = "Line ";
  strcpy( value, text.line( line ) );
  numStr( label + strlen( label ), line + 1 );
  strcat( label, ":" );
  input_.start( value, MaxLine, Frame::editWidth( label ) );
  for ( ;; ) {
    draw();
    frame_.editLine( label, value, input_.scroll(), input_.pos() );
    frame_.flush();
    unsigned key = keyboard_.read();
    unsigned char ch = keyAscii( key );
    if ( ch == KeyEsc ) {
      if ( added ) text.remove( line );
      editor_.rescan();
      return false;
    }
    if ( ch == KeyEnter ) break;
    input_.handle( key );
  }
  frame_.message( NULL );
  if ( strcmp( value, text.line( line ) ) && !text.replace( line, value ) ) {
    message_ = FileFull;
    error_ = true;
  }
  editor_.rescan();
  return true;
}

void LinesController::insert() {
  if ( cursor_.isEmpty() ) return;
  const LineRow row = rows_.at( cursor_.sel() );
  int file = row.file;
  if ( row.kind == RowMissing ) {
    int item = menu_.find( name_ );
    MenuEditor::Result r = file == ConfigSys ? editor_.ensureBlock( item ) : editor_.ensureSection( item );
    message_ = r == MenuEditor::Done ? "Made. Ins adds its lines." : menuEditMessage( r );
    error_ = r != MenuEditor::Done;
    return;
  }
  int at = row.line + 1;
  if ( !docs_.edit[file]->insert( at, "" ) ) {
    message_ = FileFull;
    error_ = true;
    return;
  }
  editor_.rescan();
  rebuild();
  selectLine( file, at );
  edit( file, at, true );
}

void LinesController::remove() {
  if ( cursor_.isEmpty() ) return;
  const LineRow &row = rows_.at( cursor_.sel() );
  if ( row.kind != RowLine ) {
    speaker_.beep();
    return;
  }
  docs_.edit[(int)row.file]->remove( row.line );
  editor_.rescan();
}

void LinesController::move( int delta ) {
  int other = cursor_.sel() + delta;
  if ( cursor_.isEmpty() || other < 0 || other >= rows_.count() ) return;
  const LineRow &a = rows_.at( cursor_.sel() ), &b = rows_.at( other );
  if ( a.kind != RowLine || b.kind != RowLine || a.file != b.file ) {
    speaker_.beep();
    return;
  }
  int file = a.file, line = b.line;
  docs_.edit[file]->swap( a.line, b.line );
  editor_.rescan();
  rebuild();
  selectLine( file, line );
}

void LinesController::nextFile() {
  for ( int i = 1; i <= BootFileCount; i++ ) {
    int f = ( file_ + i ) % BootFileCount;
    if ( docs_.isEditable( f ) ) {
      file_ = f;
      cursor_.place( 0, 0 );
      return;
    }
  }
}

bool LinesController::handleKey( unsigned key ) {
  unsigned char ch = keyAscii( key ), scan = keyScan( key );
  if ( ch == KeyEsc ) return false;
  if ( moveInList( cursor_, key ) ) return true;
  if ( ch == KeyEnter && !cursor_.isEmpty() ) {
    const LineRow row = rows_.at( cursor_.sel() );
    if ( row.kind == RowLine ) edit( row.file, row.line, false );
    else if ( row.kind == RowMissing ) insert();
    else speaker_.beep();
  }
  else if ( ch == KeyTab && file_ >= 0 ) nextFile();
  else if ( !isExtendedKey( key ) ) return true;
  else if ( scan == ScanIns ) insert();
  else if ( scan == ScanDel ) remove();
  else if ( scan == ScanCtrlPgUp || scan == ScanCtrlUp ) move( -1 );
  else if ( scan == ScanCtrlPgDn || scan == ScanCtrlDown ) move( 1 );
  else if ( scan == ScanF1 ) help_.run();
  else if ( scan == ScanF4 && diffs_.showEdits( false ) == DiffController::NoChanges )
    message_ = "Nothing is changed yet.";
  return true;
}
