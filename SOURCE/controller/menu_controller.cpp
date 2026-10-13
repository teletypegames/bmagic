#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "backup_controller.h"
#include "diff_controller.h"
#include "form_controller.h"
#include "help_controller.h"
#include "key_codes.h"
#include "lines_controller.h"
#include "list_keys.h"
#include "menu_controller.h"
#include "../lib/core/str.h"
#include "../lib/platform/keyboard.h"
#include "../lib/platform/speaker.h"
#include "../lib/ui/keys.h"
#include "../model/boot_docs.h"
#include "../model/boot_menu.h"
#include "../model/menu_editor.h"
#include "../model/startup_options.h"
#include "../model/text_file.h"
#include "../service/boot_service.h"
#include "../view/frame.h"
#include "../view/menu_view.h"

static const char NoSuchItem[] = "No item has that block name";

MenuController::MenuController( MenuView &view, Frame &frame, Keyboard &keyboard, Speaker &speaker, BootDocs &docs,
                                BootMenu &menu, MenuEditor &editor, BootService &boot, BackupList &sets,
                                FormController &form, LinesController &lines, DiffController &diffs,
                                BackupController &backups, HelpController &help, char drive )
  : view_( view ), frame_( frame ), keyboard_( keyboard ), speaker_( speaker ), docs_( docs ), menu_( menu ),
    editor_( editor ), boot_( boot ), sets_( sets ), form_( form ), lines_( lines ), diffs_( diffs ),
    backups_( backups ), help_( help ), drive_( drive ), message_( NULL ), error_( false ), written_( false ) {
  cursor_.setRows( BootMenu::MaxItems );
}

void MenuController::say( const char *text, bool error ) {
  message_ = text;
  error_ = error;
}

void MenuController::report( int result ) {
  if ( result != MenuEditor::Done ) say( menuEditMessage( (MenuEditor::Result)result ), true );
}

int MenuController::run() {
  editor_.rescan();
  for ( ;; ) {
    cursor_.setCount( menu_.count );
    cursor_.clamp();
    unsigned changed = 0;
    for ( int f = 0; f < BootFileCount; f++ ) {
      if ( boot_.isChanged( f ) ) changed |= 1 << f;
    }
    view_.draw( menu_, docs_, changed, menu_.count ? cursor_.sel() : -1, drive_, sets_ );
    if ( error_ ) frame_.error( message_ );
    else frame_.message( message_ );
    frame_.flush();
    say( NULL, false );
    if ( !handleKey( keyboard_.read() ) ) break;
    editor_.rescan();
  }
  return written_ ? 0 : 1;
}

bool MenuController::handleKey( unsigned key ) {
  unsigned char ch = keyAscii( key ), scan = keyScan( key );
  if ( ch == KeyEsc ) return !quit();
  if ( moveInList( cursor_, key ) ) return true;
  if ( ch == KeyEnter ) {
    if ( hasItem() && canEdit() ) lines_.runItem( cursor_.sel() );
    return true;
  }
  if ( !isExtendedKey( key ) ) return true;
  switch ( scan ) {
    case ScanIns: newItem(); break;
    case ScanDel: deleteItem(); break;
    case ScanCtrlPgUp:
    case ScanCtrlUp: moveItem( -1 ); break;
    case ScanCtrlPgDn:
    case ScanCtrlDown: moveItem( 1 ); break;
    default: return handleFunctionKey( scan );
  }
  return true;
}

bool MenuController::handleFunctionKey( unsigned char scan ) {
  switch ( scan ) {
    case ScanF1: help_.run(); break;
    case ScanF2: save(); break;
    case ScanF3: editItem(); break;
    case ScanF4:
      if ( diffs_.showEdits( false ) == DiffController::NoChanges ) say( "Nothing is changed yet.", false );
      break;
    case ScanF5: makeDefault(); break;
    case ScanF6: menuOptions(); break;
    case ScanF7: startupOptions(); break;
    case ScanF8:
      if ( docs_.isEditable( ConfigSys ) ) lines_.runFile( ConfigSys );
      else if ( docs_.isEditable( AutoexecBat ) ) lines_.runFile( AutoexecBat );
      else say( "CONFIG.SYS and AUTOEXEC.BAT cannot be edited.", true );
      break;
    case ScanF9:
      if ( backups_.run() ) written_ = true;
      break;
  }
  return true;
}

bool MenuController::quit() {
  if ( !boot_.isChanged() ) return true;
  frame_.error( "Save the changes?  Y: review and save   N: quit without saving   Esc: stay" );
  frame_.flush();
  unsigned char answer = keyAscii( keyboard_.read() );
  if ( answer == 'n' || answer == 'N' ) return true;
  if ( answer != 'y' && answer != 'Y' ) return false;
  save();
  return !boot_.isChanged();
}

static char *putSet( char *d, int set ) {
  *d++ = (char)( '0' + set / 100 % 10 );
  *d++ = (char)( '0' + set / 10 % 10 );
  *d++ = (char)( '0' + set % 10 );
  *d = 0;
  return d;
}

void MenuController::save() {
  if ( !boot_.isChanged() ) {
    say( "Nothing to save.", false );
    return;
  }
  if ( diffs_.showEdits( true ) != DiffController::Confirmed ) return;
  int set, failed;
  BootService::Save result = boot_.save( set, failed );
  if ( result == BootService::BackupFailed ) {
    say( "Not saved: the backup could not be made (the disk is full or write-protected?)", true );
    return;
  }
  written_ = true;
  char *d = text_;
  if ( result == BootService::WriteFailed ) {
    *d++ = drive_;
    strcpy( d, ":\\" );
    strcat( d, bootFileName( failed ) );
    strcat( d, " could not be written. Backup " );
    putSet( d + strlen( d ), set );
    strcat( d, " has the files as they were." );
    say( text_, true );
    return;
  }
  strcpy( d, "Saved. Backup " );
  putSet( d + strlen( d ), set );
  strcat( d, " has them as they were. Restart the computer to use them." );
  say( text_, false );
}

bool MenuController::canEdit() {
  if ( docs_.isEditable( ConfigSys ) && docs_.isEditable( AutoexecBat ) ) return true;
  say( "CONFIG.SYS or AUTOEXEC.BAT cannot be edited (see Files).", true );
  return false;
}

bool MenuController::hasItem() {
  if ( menu_.count ) return true;
  speaker_.beep();
  return false;
}

/* The value without the blanks around it, in place. */
static void trimValue( char *s ) {
  char *t = trim( s );
  if ( t != s ) memmove( s, t, strlen( t ) + 1 );
}

void MenuController::newItem() {
  if ( !canEdit() ) return;
  if ( menu_.count == BootMenu::MaxItems ) {
    report( MenuEditor::MenuFull );
    return;
  }
  char name[MaxBlock + 1] = "", text[MaxMenuText + 1] = "", copy[MaxBlock + 1] = "";
  FormField fields[3] = { { "Block name", name, MaxNewBlock,
                            "1-8 letters or digits: the [NAME] block and the :NAME label" },
                          { "Menu text", text, MaxMenuText, "What the boot menu shows (empty: the block name)" },
                          { "Copy of", copy, MaxBlock, "An item whose lines the new one starts with (empty: none)" } };
  const char *error = NULL;
  int field = 0;
  for ( ;; ) {
    if ( !form_.run( "New menu item", fields, 3, error, field ) ) return;
    trimValue( name );
    trimValue( text );
    trimValue( copy );
    int copyOf = copy[0] ? menu_.find( copy ) : -1;
    if ( copy[0] && copyOf < 0 ) {
      error = NoSuchItem;
      field = 2;
      continue;
    }
    MenuEditor::Result r = editor_.add( name, text, copyOf );
    if ( r == MenuEditor::Done ) break;
    error = menuEditMessage( r );
    field = 0;
  }
  cursor_.select( menu_.find( name ) );
  say( "Added. Enter edits its lines in CONFIG.SYS and AUTOEXEC.BAT.", false );
}

void MenuController::editItem() {
  if ( !hasItem() || !canEdit() ) return;
  int item = cursor_.sel();
  char name[MaxBlock + 1], text[MaxMenuText + 1];
  strcpy( name, menu_.items[item].name );
  strcpy( text, menu_.items[item].text );
  FormField fields[2] = { { "Block name", name, MaxNewBlock,
                            "Renames the MENUITEM, [NAME], :NAME, MENUDEFAULT and INCLUDE lines" },
                          { "Menu text", text, MaxMenuText, "What the boot menu shows (empty: the block name)" } };
  const char *error = NULL;
  int field = 0;
  for ( ;; ) {
    if ( !form_.run( "Menu item", fields, 2, error, field ) ) return;
    trimValue( name );
    trimValue( text );
    MenuEditor::Result r = MenuEditor::Done;
    if ( cmpi( name, menu_.items[item].name ) ) r = editor_.rename( item, name );
    if ( r == MenuEditor::Done && strcmp( text, menu_.items[item].text ) ) r = editor_.setText( item, text );
    if ( r == MenuEditor::Done ) return;
    error = menuEditMessage( r );
    field = r == MenuEditor::FileFull ? 1 : 0;
  }
}

void MenuController::deleteItem() {
  if ( !hasItem() || !canEdit() ) return;
  const char *name = menu_.items[cursor_.sel()].name;
  strcpy( text_, "Delete " );
  strncat( text_, name, 20 );
  strcat( text_, " with its block in CONFIG.SYS and its section in AUTOEXEC.BAT? (Y/N)" );
  frame_.error( text_ );
  frame_.flush();
  unsigned char answer = keyAscii( keyboard_.read() );
  if ( answer != 'y' && answer != 'Y' ) return;
  report( editor_.remove( cursor_.sel() ) );
}

void MenuController::moveItem( int delta ) {
  if ( !hasItem() || !canEdit() ) return;
  int other = cursor_.sel() + delta;
  if ( other < 0 || other >= menu_.count ) {
    speaker_.beep();
    return;
  }
  editor_.move( cursor_.sel(), delta );
  cursor_.select( other );
}

void MenuController::makeDefault() {
  if ( !hasItem() || !canEdit() ) return;
  report( editor_.setDefault( cursor_.sel(), menu_.timeout ) );
}

void MenuController::menuOptions() {
  if ( !canEdit() ) return;
  if ( !menu_.hasMenu() ) {
    report( MenuEditor::NoMenu );
    return;
  }
  char def[MaxBlock + 1], timeout[4] = "", color[16], numlock[8];
  strcpy( def, menu_.defaultName );
  if ( menu_.timeout >= 0 ) numStr( timeout, menu_.timeout );
  strcpy( color, menu_.color );
  strcpy( numlock, menu_.numlock );
  FormField fields[4] = { { "Default", def, MaxBlock, "The block of the item that starts when the time runs out" },
                          { "Timeout", timeout, 2, "Seconds, 0-90 (empty: the menu waits for a key)" },
                          { "Color", color, 7, "Text[,background]: 0-15, for example 15,1 (empty: as DOS has it)" },
                          { "NumLock", numlock, 3, "ON or OFF at boot (empty: as the BIOS sets it)" } };
  const char *error = NULL;
  int field = 0;
  for ( ;; ) {
    if ( !form_.run( "Boot menu", fields, 4, error, field ) ) return;
    trimValue( def );
    trimValue( timeout );
    int item = def[0] ? menu_.find( def ) : -1;
    int seconds = timeout[0] ? atoi( timeout ) : BootMenu::NoTimeout;
    bool digits = true;
    for ( const char *s = timeout; *s; s++ ) digits = digits && *s >= '0' && *s <= '9';
    MenuEditor::Result r = MenuEditor::Done;
    error = NULL;
    if ( def[0] && item < 0 ) error = NoSuchItem;
    else if ( !def[0] && timeout[0] ) error = "A timeout needs a default item";
    else if ( !digits ) r = MenuEditor::BadTimeout;
    if ( !error && r == MenuEditor::Done ) r = editor_.setDefault( item, seconds );
    field = 1;
    if ( !error && r == MenuEditor::Done ) {
      r = editor_.setColor( color );
      field = 2;
    }
    if ( !error && r == MenuEditor::Done ) {
      r = editor_.setNumlock( numlock );
      field = 3;
    }
    if ( !error && r == MenuEditor::Done ) return;
    if ( !error ) error = menuEditMessage( r );
    else field = 0;
  }
}

void MenuController::startupOptions() {
  TextFile &msdos = *docs_.edit[MsdosSys];
  if ( docs_.state[MsdosSys] == FileBinary ) {
    say( "MSDOS.SYS is the binary system file of MS-DOS: it has no startup options.", true );
    return;
  }
  if ( docs_.state[MsdosSys] != FileText || !StartupOptions::isWindowsFile( msdos ) ) {
    say( "There is no MSDOS.SYS of Windows 95/98 with startup options on this drive.", true );
    return;
  }
  StartupOptions options;
  options.read( msdos );
  static const char *const Hints[OptionCount] = { "1: the Windows Startup Menu at every boot, 0: only with F8",
                                                  "The Startup Menu item it starts after the delay",
                                                  "Seconds before the default item starts",
                                                  "1: start Windows, 0: stay at the command prompt",
                                                  "1: the Windows logo while it boots, 0: no logo",
                                                  "1: F5 and F8 work at boot, 0: they do not" };
  FormField fields[OptionCount];
  for ( int f = 0; f < OptionCount; f++ ) {
    FormField field = { StartupOptions::key( f ), options.value( f ), StartupOptions::MaxValue, Hints[f] };
    fields[f] = field;
  }
  const char *error = NULL;
  int field = 0;
  for ( ;; ) {
    if ( !form_.run( "Windows startup (MSDOS.SYS)", fields, OptionCount, error, field ) ) return;
    for ( int f = 0; f < OptionCount; f++ ) trimValue( options.value( f ) );
    field = options.invalidField();
    if ( field < 0 ) break;
    error = StartupOptions::isSwitch( field ) ? "0 or 1 (empty: the Windows default)"
                                              : "A number (empty: the Windows default)";
  }
  if ( !options.write( msdos ) ) say( menuEditMessage( MenuEditor::FileFull ), true );
}
