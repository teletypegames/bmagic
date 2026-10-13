#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "boot_menu.h"
#include "dos_syntax.h"
#include "limits.h"
#include "menu_editor.h"
#include "text_file.h"
#include "../lib/core/str.h"

/* Inserts lines from one place on: new text, or copies of lines of the same file
   (numbered as before the first insert). */
class Inserter {
public:
  Inserter( TextFile &file, int at ) : file_( file ), at_( at ), added_( 0 ), ok_( true ) {}
  void text( const char *s ) {
    if ( ok_ && file_.insert( at_ + added_, s ) ) added_++;
    else ok_ = false;
  }
  void copy( int line ) {
    if ( line >= at_ ) line += added_;
    if ( ok_ && file_.duplicate( at_ + added_, line ) ) added_++;
    else ok_ = false;
  }
  /* The line after the inserted ones. */
  int next() const { return at_ + added_; }
  bool ok() const { return ok_; }

private:
  TextFile &file_;
  int at_, added_;
  bool ok_;
};

/* a + b + c into out (MaxLine + 1 bytes), cut at MaxLine. */
static void join( char *out, const char *a, const char *b, const char *c ) {
  copyStr( out, a, MaxLine + 1 );
  strncat( out, b, MaxLine - strlen( out ) );
  strncat( out, c, MaxLine - strlen( out ) );
}

static void itemLine( char *out, const char *name, const char *text ) {
  join( out, "MENUITEM=", name, "" );
  if ( *text ) join( out, out, ",", text );
}

/* The last line of from..to that is not blank, plus one. */
static int bodyEnd( const TextFile &file, int from, int to ) {
  while ( to > from && isBlankLine( file.line( to - 1 ) ) ) to--;
  return to;
}

MenuEditor::MenuEditor( TextFile &config, TextFile &autoexec, BootMenu &menu )
  : config_( config ), autoexec_( autoexec ), menu_( menu ) {}

void MenuEditor::rescan() {
  menu_.scan( config_, autoexec_ );
}

static bool isNameChar( char c ) {
  return ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || c == '_' || c == '-';
}

MenuEditor::Result MenuEditor::checkName( char *name, int except ) const {
  upcaseStr( name );
  int n = strlen( name );
  if ( n == 0 || n > MaxNewBlock ) return BadName;
  for ( int i = 0; i < n; i++ ) {
    if ( !isNameChar( name[i] ) ) return BadName;
  }
  if ( strcmp( name, "MENU" ) == 0 || strcmp( name, "COMMON" ) == 0 ) return BadName;
  for ( int i = 0; i < menu_.count; i++ ) {
    if ( i != except && sameLabel( menu_.items[i].name, name ) ) return NameTaken;
  }
  if ( except >= 0 && sameLabel( menu_.items[except].name, name ) ) return Done;
  int end;
  if ( BootMenu::findBlock( config_, name, end ) >= 0 || BootMenu::findLabel( autoexec_, name ) >= 0 ) return NameTaken;
  return Done;
}

/* A CONFIG.SYS line before the first block that is not blank: those need [COMMON]. */
static bool hasLinesBeforeBlocks( const TextFile &config ) {
  char name[MaxBlock + 1];
  for ( int i = 0; i < config.count(); i++ ) {
    if ( sectionName( config.line( i ), name, sizeof( name ) ) ) return false;
    if ( !isBlankLine( config.line( i ) ) ) return true;
  }
  return false;
}

MenuEditor::Result MenuEditor::add( char *name, const char *text, int copyOf ) {
  Result r = checkName( name, -1 );
  if ( r != Done ) return r;
  if ( menu_.count == BootMenu::MaxItems ) return MenuFull;
  char line[MaxLine + 1];
  itemLine( line, name, text );
  bool ok;
  if ( menu_.hasMenu() ) {
    int at = menu_.lastItemLine >= 0 ? menu_.lastItemLine + 1 : menu_.menuLine + 1;
    ok = config_.insert( at, line );
  }
  else {
    bool common = hasLinesBeforeBlocks( config_ );
    Inserter top( config_, 0 );
    top.text( "[MENU]" );
    top.text( line );
    if ( config_.count() > 2 ) top.text( "" );
    if ( common ) top.text( "[COMMON]" );
    ok = top.ok();
  }
  if ( !ok ) return FileFull;
  /* the copied item keeps its index: the new MENUITEM line comes after it */
  rescan();
  ok = insertBlock( name, copyOf );
  rescan();
  int item = menu_.find( name );
  ok = ( menu_.gotoLine < 0 ? addDispatcher( copyOf, item ) : insertSection( name, copyOf ) ) && ok;
  rescan();
  return ok ? Done : FileFull;
}

bool MenuEditor::insertBlock( const char *name, int copyOf ) {
  int at = -1;
  for ( int i = 0; i < menu_.count; i++ ) {
    if ( menu_.items[i].blockEnd > at ) at = menu_.items[i].blockEnd;
  }
  if ( at < 0 ) at = config_.count();
  Inserter block( config_, at );
  if ( at > 0 && !isBlankLine( config_.line( at - 1 ) ) ) block.text( "" );
  char line[MaxLine + 1];
  join( line, "[", name, "]" );
  block.text( line );
  if ( copyOf >= 0 && menu_.items[copyOf].blockLine >= 0 ) {
    const MenuItem &from = menu_.items[copyOf];
    int end = bodyEnd( config_, from.blockLine + 1, from.blockEnd );
    for ( int i = from.blockLine + 1; i < end; i++ ) block.copy( i );
  }
  if ( block.next() < config_.count() && !isBlankLine( config_.line( block.next() ) ) ) block.text( "" );
  return block.ok();
}

/* The section's lines: the label, the copied lines, GOTO to the end label if they do not
   end with a GOTO. */
static void writeSection( Inserter &section, const TextFile &batch, const BootMenu &menu, const char *name,
                          int copyOf ) {
  char line[MaxLine + 1];
  join( line, ":", name, "" );
  section.text( line );
  bool jumps = false;
  if ( copyOf >= 0 && menu.items[copyOf].labelLine >= 0 ) {
    const MenuItem &from = menu.items[copyOf];
    int end = bodyEnd( batch, from.labelLine + 1, from.labelEnd );
    jumps = end > from.labelLine + 1 && gotoTarget( batch.line( end - 1 ), line, sizeof( line ) );
    for ( int i = from.labelLine + 1; i < end; i++ ) section.copy( i );
  }
  if ( !jumps ) {
    join( line, "GOTO ", menu.endLabel, "" );
    section.text( line );
  }
}

bool MenuEditor::insertSection( const char *name, int copyOf ) {
  if ( menu_.endLabelLine < 0 ) {
    char line[MaxLine + 1];
    join( line, ":", menu_.endLabel, "" );
    if ( !autoexec_.insert( autoexec_.count(), line ) ) return false;
    rescan();
  }
  int at = menu_.endLabelLine > menu_.gotoLine ? menu_.endLabelLine : autoexec_.count();
  Inserter section( autoexec_, at );
  writeSection( section, autoexec_, menu_, name, copyOf );
  if ( at > 0 && isBlankLine( autoexec_.line( at - 1 ) ) ) section.text( "" );
  return section.ok();
}

bool MenuEditor::addDispatcher( int copyOf, int item ) {
  Inserter end( autoexec_, autoexec_.count() );
  int n = autoexec_.count();
  if ( n > 0 && !isBlankLine( autoexec_.line( n - 1 ) ) ) end.text( "" );
  end.text( "GOTO %CONFIG%" );
  for ( int i = 0; i < menu_.count; i++ ) {
    if ( menu_.items[i].labelLine < 0 )
      writeSection( end, autoexec_, menu_, menu_.items[i].name, i == item ? copyOf : -1 );
  }
  if ( menu_.endLabelLine < 0 ) {
    char line[MaxLine + 1];
    join( line, ":", menu_.endLabel, "" );
    end.text( line );
  }
  return end.ok();
}

MenuEditor::Result MenuEditor::ensureBlock( int item ) {
  if ( menu_.items[item].blockLine >= 0 ) return Done;
  char name[MaxBlock + 1];
  strcpy( name, menu_.items[item].name );
  bool ok = insertBlock( name, -1 );
  rescan();
  return ok ? Done : FileFull;
}

MenuEditor::Result MenuEditor::ensureSection( int item ) {
  if ( menu_.items[item].labelLine >= 0 ) return Done;
  char name[MaxBlock + 1];
  strcpy( name, menu_.items[item].name );
  bool ok = menu_.gotoLine < 0 ? addDispatcher( -1, item ) : insertSection( name, -1 );
  rescan();
  return ok ? Done : FileFull;
}

MenuEditor::Result MenuEditor::rename( int item, char *name ) {
  Result r = checkName( name, item );
  if ( r != Done ) return r;
  MenuItem &it = menu_.items[item];
  char old[MaxBlock + 1], line[MaxLine + 1];
  strcpy( old, it.name );
  bool ok = true;
  itemLine( line, name, it.text );
  ok = config_.replace( it.itemLine, line ) && ok;
  if ( it.blockLine >= 0 ) {
    join( line, "[", name, "]" );
    ok = config_.replace( it.blockLine, line ) && ok;
  }
  if ( menu_.defaultItem() == item ) {
    join( line, "MENUDEFAULT=", name, "" );
    if ( menu_.timeout != BootMenu::NoTimeout ) {
      char digits[8] = ",";
      numStr( digits + 1, menu_.timeout );
      join( line, line, digits, "" );
    }
    ok = config_.replace( menu_.defaultLine, line ) && ok;
  }
  for ( int i = 0; i < config_.count(); i++ ) {
    const char *v = configValue( config_.line( i ), "INCLUDE" );
    if ( v && cmpi( v, old ) == 0 ) {
      join( line, "INCLUDE=", name, "" );
      ok = config_.replace( i, line ) && ok;
    }
  }
  if ( it.labelLine >= 0 ) {
    join( line, ":", name, "" );
    ok = autoexec_.replace( it.labelLine, line ) && ok;
  }
  rescan();
  return ok ? Done : FileFull;
}

MenuEditor::Result MenuEditor::setText( int item, const char *text ) {
  char line[MaxLine + 1];
  itemLine( line, menu_.items[item].name, text );
  bool ok = config_.replace( menu_.items[item].itemLine, line );
  rescan();
  return ok ? Done : FileFull;
}

void MenuEditor::removeLines( TextFile &file, int from, int to ) {
  while ( to > from ) file.remove( --to );
}

void MenuEditor::removeIncludes( const char *name ) {
  for ( int i = config_.count() - 1; i >= 0; i-- ) {
    const char *v = configValue( config_.line( i ), "INCLUDE" );
    if ( v && cmpi( v, name ) == 0 ) config_.remove( i );
  }
}

MenuEditor::Result MenuEditor::remove( int item ) {
  char name[MaxBlock + 1];
  strcpy( name, menu_.items[item].name );
  const MenuItem &it = menu_.items[item];
  if ( it.labelLine > menu_.gotoLine && menu_.gotoLine >= 0 ) removeLines( autoexec_, it.labelLine, it.labelEnd );
  if ( it.blockLine >= 0 ) removeLines( config_, it.blockLine, it.blockEnd );
  removeIncludes( name );
  rescan();
  if ( menu_.defaultItem() == menu_.find( name ) ) config_.remove( menu_.defaultLine );
  rescan();
  config_.remove( menu_.items[menu_.find( name )].itemLine );
  rescan();
  if ( menu_.count == 0 && menu_.lastItemLine < 0 && menu_.submenus == 0 ) {
    removeLines( config_, menu_.menuLine, menu_.menuEnd );
    if ( menu_.gotoLine >= 0 ) autoexec_.remove( menu_.gotoLine );
    rescan();
  }
  return Done;
}

MenuEditor::Result MenuEditor::move( int item, int delta ) {
  int other = item + delta;
  if ( other < 0 || other >= menu_.count ) return Done;
  config_.swap( menu_.items[item].itemLine, menu_.items[other].itemLine );
  rescan();
  return Done;
}

/* Where a new MENUDEFAULT, MENUCOLOR or NUMLOCK line goes: after the items. */
int MenuEditor::optionLine() const {
  int at = menu_.lastItemLine >= 0 ? menu_.lastItemLine : menu_.menuLine;
  if ( menu_.defaultLine > at ) at = menu_.defaultLine;
  if ( menu_.colorLine > at ) at = menu_.colorLine;
  if ( menu_.numlockLine > at ) at = menu_.numlockLine;
  return at + 1;
}

/* KEY=value in place of line, a new one after the items, or none when value is empty. */
bool MenuEditor::setMenuLine( int &line, const char *key, const char *value ) {
  char text[MaxLine + 1];
  join( text, key, "=", value );
  bool ok = true;
  if ( !*value ) {
    if ( line >= 0 ) config_.remove( line );
  }
  else if ( line >= 0 ) ok = config_.replace( line, text );
  else ok = config_.insert( optionLine(), text );
  rescan();
  return ok;
}

MenuEditor::Result MenuEditor::setDefault( int item, int timeout ) {
  if ( !menu_.hasMenu() ) return NoMenu;
  if ( timeout > 90 ) return BadTimeout;
  char value[MaxLine + 1] = "";
  if ( item >= 0 ) {
    strcpy( value, menu_.items[item].name );
    if ( timeout >= 0 ) {
      strcat( value, "," );
      numStr( value + strlen( value ), timeout );
    }
  }
  return setMenuLine( menu_.defaultLine, "MENUDEFAULT", value ) ? Done : FileFull;
}

/* A color number 0-15 at s: after it, or NULL. */
static const char *colorNumber( const char *s ) {
  s = skipBlank( s );
  if ( *s < '0' || *s > '9' ) return NULL;
  int n = 0;
  while ( *s >= '0' && *s <= '9' ) n = n * 10 + *s++ - '0';
  return n <= 15 ? skipBlank( s ) : NULL;
}

MenuEditor::Result MenuEditor::setColor( const char *color ) {
  if ( !menu_.hasMenu() ) return NoMenu;
  const char *s = skipBlank( color );
  if ( *s ) {
    s = colorNumber( s );
    if ( s && *s == ',' ) s = colorNumber( s + 1 );
    if ( s == NULL || *s ) return BadColor;
  }
  char value[16];
  int n = 0;
  for ( s = color; *s && n < 15; s++ ) {
    if ( *s != ' ' && *s != '\t' ) value[n++] = *s;
  }
  value[n] = 0;
  return setMenuLine( menu_.colorLine, "MENUCOLOR", value ) ? Done : FileFull;
}

MenuEditor::Result MenuEditor::setNumlock( const char *numlock ) {
  if ( !menu_.hasMenu() ) return NoMenu;
  char value[8];
  copyStr( value, skipBlank( numlock ), sizeof( value ) );
  trim( value );
  upcaseStr( value );
  if ( *value && strcmp( value, "ON" ) && strcmp( value, "OFF" ) ) return BadNumlock;
  return setMenuLine( menu_.numlockLine, "NUMLOCK", value ) ? Done : FileFull;
}

const char *menuEditMessage( MenuEditor::Result result ) {
  switch ( result ) {
    case MenuEditor::Done: return "";
    case MenuEditor::BadName: return "The name: 1-8 letters, digits, _ or - (not MENU or COMMON)";
    case MenuEditor::NameTaken: return "That name is taken (labels count 8 characters)";
    case MenuEditor::MenuFull: return "The menu has 9 items already (DOS shows no more)";
    case MenuEditor::NoMenu: return "There is no boot menu yet: Ins adds the first item";
    case MenuEditor::BadTimeout: return "The timeout: 0-90 seconds, empty: the menu waits";
    case MenuEditor::BadColor: return "The color: text[,background], numbers 0-15 (e.g. 15,1)";
    case MenuEditor::BadNumlock: return "NumLock: ON, OFF, or empty";
    case MenuEditor::FileFull: return "The file is full (500 lines, 16 KB)";
  }
  return "";
}
