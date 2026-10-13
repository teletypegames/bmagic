#include <string.h>

#include "fakes.h"
#include "../../lib/testing/host/minitest.h"
#include "../../model/backup_list.h"
#include "../../model/boot_file.h"
#include "../../model/boot_menu.h"
#include "../../model/dos_syntax.h"
#include "../../model/line_rows.h"
#include "../../model/menu_editor.h"
#include "../../model/startup_options.h"
#include "../../model/text_diff.h"
#include "../../model/text_file.h"

static TextFile *text( const char *s ) {
  TextFile *t = new TextFile;
  t->load( s, strlen( s ) );
  return t;
}

static const char *written( const TextFile &t ) {
  static char out[TextFile::OutSize + 1];
  unsigned n = t.write( out );
  out[n] = 0;
  return out;
}

/* Two files with their menu and editor. */
struct Boot {
  Boot( const char *config, const char *autoexec )
    : cfg( text( config ) ), bat( text( autoexec ) ), editor( *cfg, *bat, menu ) {
    editor.rescan();
  }
  ~Boot() {
    delete cfg;
    delete bat;
  }
  TextFile *cfg, *bat;
  BootMenu menu;
  MenuEditor editor;
};

TEST( textFileKeepsTheBytes ) {
  const char *files[] = { "A\r\nB\r\n", "A\nB", "A\r\n\r\nB\r\n\x1A", "", "X" };
  for ( int i = 0; i < 5; i++ ) {
    TextFile *t = text( files[i] );
    CHECK_STR( written( *t ), files[i] );
    delete t;
  }
}

TEST( textFileEditsLines ) {
  TextFile *t = text( "one\ntwo\n" );
  CHECK_INT( t->count(), 2 );
  CHECK( t->insert( 1, "middle" ) );
  CHECK( t->replace( 0, "the first line, longer" ) );
  t->remove( 2 );
  CHECK_STR( written( *t ), "the first line, longer\nmiddle\n" );
  CHECK( t->duplicate( 0, 1 ) );
  t->swap( 1, 2 );
  CHECK_STR( written( *t ), "middle\nmiddle\nthe first line, longer\n" );
  delete t;
}

TEST( textFileCompactsItsPool ) {
  TextFile *t = text( "x" );
  char line[101];
  memset( line, 'a', 100 );
  line[100] = 0;
  for ( int i = 0; i < 1000; i++ ) {
    line[0] = (char)( 'a' + i % 26 );
    CHECK( t->replace( 0, line ) );
  }
  CHECK_INT( (int)strlen( t->line( 0 ) ), 100 );
  CHECK( t->line( 0 )[0] == 'a' + 999 % 26 );
  delete t;
}

TEST( textFileIsBinary ) {
  CHECK( TextFile::isBinary( "ab\0c", 4 ) );
  CHECK( !TextFile::isBinary( "abc", 3 ) );
}

TEST( syntaxReadsConfigAndBatchLines ) {
  char name[20];
  CHECK( sectionName( "  [ Normal ]", name, sizeof( name ) ) );
  CHECK_STR( name, "Normal" );
  CHECK_STR( configValue( "menuitem = A,B", "MENUITEM" ), "A,B" );
  CHECK_STR( configValue( "MENUITEM A", "MENUITEM" ), "A" );
  CHECK( configValue( "MENUITEMX=A", "MENUITEM" ) == NULL );
  CHECK( labelName( ":games  rem", name, sizeof( name ) ) );
  CHECK_STR( name, "games" );
  CHECK( !labelName( ":: a comment", name, sizeof( name ) ) );
  CHECK( sameLabel( "LONGNAME1", "longname2" ) );
  CHECK( !sameLabel( "ABC", "ABCD" ) );
  CHECK( isGotoConfig( "  goto %config%" ) );
  CHECK( isGotoConfig( "@GOTO :%CONFIG%" ) );
  CHECK( !isGotoConfig( "GOTOEND" ) );
  CHECK( gotoTarget( "goto end", name, sizeof( name ) ) );
  CHECK_STR( name, "end" );
}

TEST( menuIsMapped ) {
  Boot b( MenuConfig, MenuAutoexec );
  BootMenu &m = b.menu;
  CHECK( m.hasMenu() );
  CHECK_INT( m.count, 2 );
  CHECK_STR( m.items[0].name, "NORMAL" );
  CHECK_STR( m.items[1].text, "Games with EMS" );
  CHECK_INT( m.defaultItem(), 0 );
  CHECK_INT( m.timeout, 10 );
  CHECK_INT( m.items[0].blockLine, 9 );
  CHECK_INT( m.items[0].blockEnd, 12 );
  CHECK_INT( m.items[1].blockEnd, 15 );
  CHECK_INT( m.gotoLine, 2 );
  CHECK_INT( m.items[1].labelLine, 6 );
  CHECK_INT( m.items[1].labelEnd, 9 );
  CHECK_STR( m.endLabel, "END" );
  CHECK_INT( m.endLabelLine, 9 );
  CHECK( !m.labelMissing( 0 ) );
}

TEST( addItemCopiesAnother ) {
  Boot b( MenuConfig, MenuAutoexec );
  char name[] = "safe";
  CHECK_INT( b.editor.add( name, "Safe mode", 1 ), MenuEditor::Done );
  CHECK_STR( name, "SAFE" );
  CHECK_STR( written( *b.cfg ), "[MENU]\r\n"
                                "MENUITEM=NORMAL,Normal start\r\n"
                                "MENUITEM=GAMES,Games with EMS\r\n"
                                "MENUITEM=SAFE,Safe mode\r\n"
                                "MENUDEFAULT=NORMAL,10\r\n"
                                "\r\n"
                                "[COMMON]\r\n"
                                "DOS=HIGH,UMB\r\n"
                                "FILES=40\r\n"
                                "\r\n"
                                "[NORMAL]\r\n"
                                "DEVICE=C:\\DOS\\HIMEM.SYS\r\n"
                                "\r\n"
                                "[GAMES]\r\n"
                                "DEVICE=C:\\DOS\\HIMEM.SYS\r\n"
                                "DEVICE=C:\\DOS\\EMM386.EXE RAM\r\n"
                                "\r\n"
                                "[SAFE]\r\n"
                                "DEVICE=C:\\DOS\\HIMEM.SYS\r\n"
                                "DEVICE=C:\\DOS\\EMM386.EXE RAM\r\n" );
  CHECK_STR( written( *b.bat ), "@ECHO OFF\r\n"
                                "PROMPT $P$G\r\n"
                                "GOTO %CONFIG%\r\n"
                                ":NORMAL\r\n"
                                "SMARTDRV\r\n"
                                "GOTO END\r\n"
                                ":GAMES\r\n"
                                "MOUSE\r\n"
                                "GOTO END\r\n"
                                ":SAFE\r\n"
                                "MOUSE\r\n"
                                "GOTO END\r\n"
                                ":END\r\n" );
  CHECK_INT( b.menu.count, 3 );
}

TEST( addItemChecksTheName ) {
  Boot b( MenuConfig, MenuAutoexec );
  char bad[] = "TWO WORDS", longer[] = "NORMAL12", common[] = "common", taken[] = "games";
  CHECK_INT( b.editor.add( bad, "", -1 ), MenuEditor::BadName );
  CHECK_INT( b.editor.add( common, "", -1 ), MenuEditor::BadName );
  CHECK_INT( b.editor.add( taken, "", -1 ), MenuEditor::NameTaken );
  CHECK_INT( b.editor.add( longer, "", -1 ), MenuEditor::Done );
  char clash[] = "NORMAL1";
  CHECK_INT( b.editor.add( clash, "", -1 ), MenuEditor::Done );
  char end[] = "END";
  CHECK_INT( b.editor.add( end, "", -1 ), MenuEditor::NameTaken );
}

TEST( firstItemStartsTheMenu ) {
  Boot b( PlainConfig, PlainAutoexec );
  CHECK( !b.menu.hasMenu() );
  char name[] = "DOS";
  CHECK_INT( b.editor.add( name, "Plain DOS", -1 ), MenuEditor::Done );
  CHECK_STR( written( *b.cfg ), "[MENU]\r\n"
                                "MENUITEM=DOS,Plain DOS\r\n"
                                "\r\n"
                                "[COMMON]\r\n"
                                "DEVICE=C:\\DOS\\HIMEM.SYS\r\n"
                                "DOS=HIGH\r\n"
                                "FILES=30\r\n"
                                "\r\n"
                                "[DOS]\r\n" );
  CHECK_STR( written( *b.bat ), "@ECHO OFF\r\n"
                                "PATH C:\\DOS\r\n"
                                "\r\n"
                                "GOTO %CONFIG%\r\n"
                                ":DOS\r\n"
                                "GOTO END\r\n"
                                ":END\r\n" );
  char second[] = "WIN";
  CHECK_INT( b.editor.add( second, "", 0 ), MenuEditor::Done );
  CHECK_STR( written( *b.bat ), "@ECHO OFF\r\n"
                                "PATH C:\\DOS\r\n"
                                "\r\n"
                                "GOTO %CONFIG%\r\n"
                                ":DOS\r\n"
                                "GOTO END\r\n"
                                ":WIN\r\n"
                                "GOTO END\r\n"
                                ":END\r\n" );
  CHECK_INT( b.menu.items[1].blockLine, 11 );
}

TEST( firstItemOfEmptyFiles ) {
  Boot b( "", "" );
  char name[] = "ONE";
  CHECK_INT( b.editor.add( name, "", -1 ), MenuEditor::Done );
  CHECK_STR( written( *b.cfg ), "[MENU]\r\nMENUITEM=ONE\r\n\r\n[ONE]\r\n" );
  CHECK_STR( written( *b.bat ), "GOTO %CONFIG%\r\n:ONE\r\nGOTO END\r\n:END\r\n" );
}

TEST( renameChangesEveryReference ) {
  Boot b( "[MENU]\nMENUITEM=A,First\nMENUDEFAULT=A,5\n[COMMON]\n[A]\nX=1\n[B]\nINCLUDE=A\n",
          "GOTO %CONFIG%\n:A\nGOTO END\n:END\n" );
  char name[] = "first";
  CHECK_INT( b.editor.rename( 0, name ), MenuEditor::Done );
  CHECK_STR( written( *b.cfg ), "[MENU]\nMENUITEM=FIRST,First\nMENUDEFAULT=FIRST,5\n[COMMON]\n[FIRST]\nX=1\n[B]\n"
                                "INCLUDE=FIRST\n" );
  CHECK_STR( written( *b.bat ), "GOTO %CONFIG%\n:FIRST\nGOTO END\n:END\n" );
  CHECK_INT( b.editor.setText( 0, "The first" ), MenuEditor::Done );
  CHECK_STR( b.menu.items[0].text, "The first" );
}

TEST( removeTakesTheItemsLines ) {
  Boot b( MenuConfig, MenuAutoexec );
  CHECK_INT( b.editor.remove( 0 ), MenuEditor::Done );
  CHECK_STR( written( *b.cfg ), "[MENU]\r\n"
                                "MENUITEM=GAMES,Games with EMS\r\n"
                                "\r\n"
                                "[COMMON]\r\n"
                                "DOS=HIGH,UMB\r\n"
                                "FILES=40\r\n"
                                "\r\n"
                                "[GAMES]\r\n"
                                "DEVICE=C:\\DOS\\HIMEM.SYS\r\n"
                                "DEVICE=C:\\DOS\\EMM386.EXE RAM\r\n" );
  CHECK_STR( written( *b.bat ),
             "@ECHO OFF\r\nPROMPT $P$G\r\nGOTO %CONFIG%\r\n:GAMES\r\nMOUSE\r\nGOTO END\r\n:END\r\n" );
  CHECK_INT( b.editor.remove( 0 ), MenuEditor::Done );
  CHECK( !b.menu.hasMenu() );
  CHECK_STR( written( *b.cfg ), "[COMMON]\r\nDOS=HIGH,UMB\r\nFILES=40\r\n\r\n" );
  CHECK_STR( written( *b.bat ), "@ECHO OFF\r\nPROMPT $P$G\r\n:END\r\n" );
}

TEST( moveSwapsTheMenuLines ) {
  Boot b( MenuConfig, MenuAutoexec );
  CHECK_INT( b.editor.move( 1, -1 ), MenuEditor::Done );
  CHECK_STR( b.menu.items[0].name, "GAMES" );
  CHECK_STR( b.cfg->line( 1 ), "MENUITEM=GAMES,Games with EMS" );
  CHECK_INT( b.menu.defaultItem(), 1 );
}

TEST( menuSettingsAreWritten ) {
  Boot b( MenuConfig, MenuAutoexec );
  CHECK_INT( b.editor.setDefault( 1, -1 ), MenuEditor::Done );
  CHECK_STR( b.cfg->line( 3 ), "MENUDEFAULT=GAMES" );
  CHECK_INT( b.editor.setDefault( 1, 91 ), MenuEditor::BadTimeout );
  CHECK_INT( b.editor.setColor( "15, 1" ), MenuEditor::Done );
  CHECK_STR( b.cfg->line( 4 ), "MENUCOLOR=15,1" );
  CHECK_INT( b.editor.setColor( "16" ), MenuEditor::BadColor );
  CHECK_INT( b.editor.setNumlock( "off" ), MenuEditor::Done );
  CHECK_STR( b.cfg->line( 5 ), "NUMLOCK=OFF" );
  CHECK_INT( b.editor.setNumlock( "maybe" ), MenuEditor::BadNumlock );
  CHECK_INT( b.editor.setColor( "" ), MenuEditor::Done );
  CHECK_INT( b.menu.colorLine, -1 );
  CHECK_INT( b.editor.setDefault( -1, -1 ), MenuEditor::Done );
  CHECK_INT( b.menu.defaultLine, -1 );
}

TEST( missingPartsAreMade ) {
  Boot b( "[MENU]\nMENUITEM=A\nMENUITEM=B\n[A]\nX=1\n", "ECHO\n" );
  CHECK_INT( b.menu.items[1].blockLine, -1 );
  CHECK( !b.menu.labelMissing( 0 ) );
  CHECK_INT( b.editor.ensureBlock( 1 ), MenuEditor::Done );
  CHECK_STR( written( *b.cfg ), "[MENU]\nMENUITEM=A\nMENUITEM=B\n[A]\nX=1\n\n[B]\n" );
  CHECK_INT( b.editor.ensureSection( 1 ), MenuEditor::Done );
  CHECK_STR( written( *b.bat ), "ECHO\n\nGOTO %CONFIG%\n:A\nGOTO END\n:B\nGOTO END\n:END\n" );
}

TEST( startupOptionsChangeOnlyTheirLines ) {
  TextFile *msdos =
      text( "[Paths]\r\nWinDir=C:\\WINDOWS\r\n[Options]\r\nBootGUI=1\r\nLogo=1\r\n;padding\r\n;xxxx\r\n" );
  CHECK( StartupOptions::isWindowsFile( *msdos ) );
  StartupOptions options;
  options.read( *msdos );
  CHECK_STR( options.value( OptBootGui ), "1" );
  strcpy( options.value( OptBootGui ), "0" );
  options.value( OptLogo )[0] = 0;
  strcpy( options.value( OptBootMenu ), "1" );
  strcpy( options.value( OptMenuDelay ), "5" );
  CHECK_INT( options.invalidField(), -1 );
  CHECK( options.write( *msdos ) );
  CHECK_STR( written( *msdos ), "[Paths]\r\nWinDir=C:\\WINDOWS\r\n[Options]\r\nBootGUI=0\r\nBootMenu=1\r\n"
                                "BootMenuDelay=5\r\n;padding\r\n;xxxx\r\n" );
  strcpy( options.value( OptLogo ), "2" );
  CHECK_INT( options.invalidField(), OptLogo );
  delete msdos;
}

TEST( diffShowsTheChangesWithContext ) {
  TextFile *a = text( "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n" );
  TextFile *b = text( "1\n2\n3\n4\n5\nsix\n7\n8\n9\n10\n11\n12\n13\n14\n15\nsixteen\n" );
  static TextDiff diff;
  diff.compare( *a, *b );
  CHECK_INT( diff.removed(), 1 );
  CHECK_INT( diff.added(), 2 );
  char kinds[40];
  int n = 0;
  for ( int i = 0; i < diff.count(); i++ ) kinds[n++] = diff.row( i ).kind;
  kinds[n] = 0;
  CHECK_STR( kinds, ".   -+   .   +" );
  CHECK_INT( diff.row( 0 ).b, 2 );
  CHECK_INT( diff.row( 4 ).a, 5 );
  CHECK_INT( diff.row( 5 ).b, 5 );
  delete a;
  delete b;
}

TEST( diffFindsInsertedBlocks ) {
  TextFile *a = text( "A\nB\nC\n\nD\n" );
  TextFile *b = text( "A\nB\nC\n\n[NEW]\nX\n\nD\n" );
  static TextDiff diff;
  diff.compare( *a, *b );
  CHECK_INT( diff.removed(), 0 );
  CHECK_INT( diff.added(), 3 );
  diff.compare( *a, *a );
  CHECK( diff.isSame() );
  delete a;
  delete b;
}

TEST( itemRowsShowBothParts ) {
  Boot b( MenuConfig, MenuAutoexec );
  static LineRows rows;
  rows.forItem( b.menu, 1, *b.cfg, *b.bat );
  CHECK_INT( rows.count(), 6 );
  CHECK_INT( rows.at( 0 ).kind, RowHeader );
  CHECK_INT( rows.at( 1 ).line, 13 );
  CHECK_INT( rows.at( 3 ).kind, RowHeader );
  CHECK_INT( rows.at( 4 ).line, 7 );
  CHECK_INT( rows.find( AutoexecBat, 8 ), 5 );
}

TEST( backupInfoRoundTrips ) {
  BackupSet set;
  memset( &set, 0, sizeof( set ) );
  set.setWhen( ( ( 2026 - 1980 ) << 9 ) | ( 10 << 5 ) | 3, ( 9 << 11 ) | ( 7 << 5 ) );
  CHECK_STR( set.date, "2026-10-03" );
  CHECK_STR( set.time, "09:07" );
  set.drive = 'C';
  strcpy( set.note, "Before saving" );
  char info[160];
  info[set.writeInfo( info )] = 0;
  BackupSet back;
  memset( &back, 0, sizeof( back ) );
  back.readInfo( info );
  CHECK_STR( back.date, "2026-10-03" );
  CHECK_STR( back.note, "Before saving" );
  CHECK( back.drive == 'C' );
  static BackupList list;
  set.number = 2;
  list.add( set );
  set.number = 7;
  list.add( set );
  CHECK_INT( list.at( 0 ).number, 7 );
  CHECK_INT( list.nextNumber(), 8 );
}
