#include <stddef.h>
#include <string.h>

#include "colors.h"
#include "frame.h"
#include "menu_view.h"
#include "../lib/core/str.h"
#include "../lib/ui/canvas.h"
#include "../model/backup_list.h"
#include "../model/boot_docs.h"
#include "../model/boot_menu.h"
#include "../model/startup_options.h"
#include "../model/text_file.h"

enum { NameCol = 6, TextCol = 17, ConfigCol = 61, AutoexecCol = 70, SettingsRow = 13, FilesRow = 16, BackupRow = 20 };

static const char Keys[] =
    "~Enter~ Lines ~Ins~ New ~Del~ Delete ~F2~ Save ~F4~ Changes ~F9~ Backups ~F1~ Help ~Esc~ Quit";

MenuView::MenuView( Frame &frame ) : frame_( frame ) {}

/* "12 lines" at pos, or what is missing. */
static int lineCount( Canvas &canvas, int pos, int lines, unsigned char attr ) {
  if ( lines < 0 ) return canvas.text( pos, "none", 8, AttrWarn );
  pos = canvas.number( pos, lines, attr );
  return canvas.text( pos, lines == 1 ? " line" : " lines", 6, attr );
}

void MenuView::drawItems( const BootMenu &menu, const BootDocs &docs, int sel ) {
  Canvas &c = frame_.canvas();
  c.textAt( 2, 2, "#", 1, AttrTitle );
  c.textAt( 2, NameCol, "Block", 10, AttrTitle );
  c.textAt( 2, TextCol, "Menu text", 40, AttrTitle );
  c.textAt( 2, ConfigCol, "CONFIG", 8, AttrTitle );
  c.textAt( 2, AutoexecCol, "AUTOEXEC", 8, AttrTitle );
  if ( !menu.hasMenu() || menu.count == 0 ) {
    c.textAt( FirstRow + 1, 4, "CONFIG.SYS has no boot menu.", 70, AttrText );
    c.textAt( FirstRow + 3, 4, "Ins adds the first item: [MENU] goes to the top of CONFIG.SYS, and the", 74, AttrDim );
    c.textAt( FirstRow + 4, 4, "lines that are there now become the [COMMON] part of every item.", 74, AttrDim );
    return;
  }
  int def = menu.defaultItem();
  for ( int i = 0; i < menu.count; i++ ) {
    const MenuItem &it = menu.items[i];
    unsigned char a = i == sel ? AttrSelected : AttrText;
    int row = FirstRow + i;
    int pos = c.put( Canvas::at( row, 1 ), ' ', a );
    pos = c.put( pos, (char)( '1' + i ), a );
    pos = c.put( pos, ' ', a );
    pos = c.put( pos, i == def ? (char)0x10 : ' ', i == sel ? a : AttrMark );
    pos = c.fillTo( pos, Canvas::at( row, NameCol ), a );
    pos = c.text( pos, it.name, TextCol - NameCol - 1, a );
    pos = c.fillTo( pos, Canvas::at( row, TextCol ), a );
    pos = c.text( pos, it.text[0] ? it.text : it.name, ConfigCol - TextCol - 1, a );
    pos = c.fillTo( pos, Canvas::at( row, ConfigCol ), a );
    lineCount( c, pos, BootMenu::bodyLines( *docs.edit[ConfigSys], it.blockLine, it.blockEnd ), a );
    pos = Canvas::at( row, AutoexecCol );
    if ( menu.labelMissing( i ) ) pos = c.text( pos, "no label", 8, i == sel ? a : AttrWarn );
    else if ( menu.gotoLine < 0 ) pos = c.text( pos, "-", 1, a );
    else pos = lineCount( c, pos, BootMenu::bodyLines( *docs.edit[AutoexecBat], it.labelLine, it.labelEnd ), a );
    c.fillTo( pos, Canvas::at( row, 79 ), a );
  }
}

static int putNumber( Canvas &c, int pos, int n, unsigned char attr ) {
  return c.number( pos, n, attr );
}

void MenuView::drawSettings( const BootMenu &menu, const BootDocs &docs ) {
  Canvas &c = frame_.canvas();
  int row = SettingsRow;
  if ( menu.hasMenu() ) {
    int pos = c.textAt( row, 2, "Default: ", 9, AttrDim );
    int def = menu.defaultItem();
    pos = c.text( pos, def >= 0 ? menu.items[def].name : "(the first item)", 20, AttrText );
    if ( menu.timeout >= 0 ) {
      pos = c.text( pos, " after ", 7, AttrDim );
      pos = putNumber( c, pos, menu.timeout, AttrText );
      pos = c.text( pos, " s", 2, AttrDim );
    }
    else pos = c.text( pos, ", waits", 7, AttrDim );
    pos = c.text( pos + 3, "Color: ", 7, AttrDim );
    pos = c.text( pos, menu.color[0] ? menu.color : "-", 15, AttrText );
    pos = c.text( pos + 3, "NumLock: ", 9, AttrDim );
    c.text( pos, menu.numlock[0] ? menu.numlock : "-", 7, AttrText );
  }
  row++;
  const char *warn = NULL;
  if ( menu.count && menu.gotoLine < 0 ) warn = "AUTOEXEC.BAT has no GOTO %CONFIG%: every item runs all of it";
  else if ( menu.submenus || menu.extraItems ) warn = "SUBMENU lines and MENUITEM lines over 9 are kept, not shown";
  else if ( !docs.isEditable( ConfigSys ) || !docs.isEditable( AutoexecBat ) )
    warn = "The menu cannot be changed: CONFIG.SYS or AUTOEXEC.BAT is not a text file to edit";
  if ( warn ) c.textAt( row, 2, warn, 76, AttrWarn );
}

static const char *stateText( const BootDocs &docs, int f ) {
  switch ( docs.state[f] ) {
    case FileMissing: return f == MsdosSys ? "none" : "none yet (saving makes it)";
    case FileBinary: return "binary (the MS-DOS system file), not edited";
    case FileTooBig: return "too big to edit (16000 bytes, 500 lines)";
    case FileUnreadable: return "cannot be read";
  }
  return NULL;
}

/* The Windows startup options of a text MSDOS.SYS. */
static int drawStartup( Canvas &c, int pos, const TextFile &msdos ) {
  if ( !StartupOptions::isWindowsFile( msdos ) ) return c.text( pos, "  (no [Options])", 20, AttrDim );
  StartupOptions options;
  options.read( msdos );
  static const int Shown[] = { OptBootMenu, OptMenuDelay, OptBootGui, OptLogo };
  for ( int i = 0; i < 4; i++ ) {
    pos = c.text( pos, "  ", 2, AttrDim );
    pos = c.text( pos, StartupOptions::key( Shown[i] ), 16, AttrDim );
    pos = c.put( pos, '=', AttrDim );
    const char *v = options.value( Shown[i] );
    pos = c.text( pos, *v ? v : "-", 3, AttrText );
  }
  return pos;
}

void MenuView::drawFiles( const BootDocs &docs, unsigned changed, char drive ) {
  Canvas &c = frame_.canvas();
  for ( int f = 0; f < BootFileCount; f++ ) {
    int row = FilesRow + f;
    int pos = c.put( Canvas::at( row, 2 ), drive, AttrText );
    pos = c.text( pos, ":\\", 2, AttrText );
    pos = c.text( pos, bootFileName( f ), 12, AttrText );
    pos = Canvas::at( row, 18 );
    const char *state = stateText( docs, f );
    if ( state ) pos = c.text( pos, state, 50, docs.state[f] == FileMissing ? AttrDim : AttrWarn );
    else pos = lineCount( c, pos, docs.edit[f]->count(), AttrDim );
    if ( ( changed >> f ) & 1 ) pos = c.text( pos, ", changed", 9, AttrMark );
    if ( f == MsdosSys && docs.state[f] == FileText ) drawStartup( c, pos, *docs.edit[f] );
  }
}

void MenuView::drawBackups( BackupList &sets ) {
  Canvas &c = frame_.canvas();
  int pos = c.textAt( BackupRow, 2, "Backups: ", 9, AttrDim );
  if ( sets.count() == 0 ) {
    c.text( pos, "none yet (saving makes one first)", 40, AttrText );
    return;
  }
  pos = c.number( pos, sets.count(), AttrText );
  pos = c.text( pos, ", the newest: ", 14, AttrDim );
  BackupSet &newest = sets.at( 0 );
  char number[8];
  numStr( number, newest.number );
  pos = c.text( pos, number, 3, AttrText );
  pos = c.text( pos + 1, newest.date, 10, AttrText );
  pos = c.text( pos + 1, newest.time, 5, AttrText );
  c.text( pos + 1, newest.note, 30, AttrDim );
}

void MenuView::draw( const BootMenu &menu, const BootDocs &docs, unsigned changed, int sel, char drive,
                     BackupList &sets ) {
  Canvas &c = frame_.canvas();
  char title[40] = "Boot Magic - boot menu of drive ";
  int n = strlen( title );
  title[n] = drive;
  strcpy( title + n + 1, ":" );
  frame_.title( title, changed ? "* changed" : NULL );
  c.box( 1, 0, 22, 79, AttrFrame );
  c.boxTitle( 1, 2, "Boot menu", 20, AttrTitle );
  drawItems( menu, docs, sel );
  c.separator( 12, 0, 79, AttrFrame );
  c.boxTitle( 12, 2, "Menu", 20, AttrTitle );
  drawSettings( menu, docs );
  c.separator( 15, 0, 79, AttrFrame );
  c.boxTitle( 15, 2, "Files", 20, AttrTitle );
  drawFiles( docs, changed, drive );
  c.separator( 19, 0, 79, AttrFrame );
  drawBackups( sets );
  frame_.keys( Keys );
}
