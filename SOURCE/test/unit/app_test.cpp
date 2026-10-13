#include <stddef.h>
#include <string.h>

#include "fakes.h"
#include "../../app/bmagic.h"
#include "../../lib/testing/host/fake_file_system.h"
#include "../../lib/testing/host/memory_screen.h"
#include "../../lib/testing/host/minitest.h"
#include "../../lib/testing/scripted_keyboard.h"

/* When the keys run out the screen is kept, then Esc and N follow until the program
   ends (N quits without saving at the question). */
class StopAtTheEnd : public ScriptEnd {
public:
  StopAtTheEnd() : screen( NULL ), calls( 0 ) { memset( rows, 0, sizeof( rows ) ); }
  unsigned scriptEnded() {
    if ( calls++ == 0 && screen ) {
      for ( int r = 0; r < 25; r++ ) strcpy( rows[r], screen->text( r ) );
    }
    return calls % 2 ? 0x011B : 0x316E;
  }
  /* A row of the kept screen that contains text, or -1. */
  int find( const char *text ) const {
    for ( int r = 0; r < 25; r++ ) {
      if ( strstr( rows[r], text ) ) return r;
    }
    return -1;
  }
  MemoryScreen *screen;
  int calls;
  char rows[25][81];
};

/* BMAGIC on a fake C: drive with the files, run with the keys: the exit code. */
struct Run {
  Run() {
    fs.makeFolder( "C:\\APPS" );
    fs.makeFolder( "C:\\APPS\\BMAGIC" );
  }
  int keys( const char *script, const char *arg = NULL ) {
    end.screen = &screen;
    ScriptedKeyboard keyboard( script, end );
    Machine m = { fs, screen, keyboard, speaker, out, system };
    char prog[] = "C:\\APPS\\BMAGIC\\BMAGIC.EXE";
    char *argv[] = { prog, (char *)arg, NULL };
    return runBmagic( m, "C:\\APPS\\BMAGIC\\", arg ? 2 : 1, argv );
  }

  FakeFileSystem fs;
  MemoryScreen screen;
  CountingSpeaker speaker;
  CapturedOutput out;
  FakeBootSystem system;
  StopAtTheEnd end;
};

/* Ins, the name, Enter, the text, Enter, Enter (no copy). */
#define NEW_DOS_ITEM "5200 44 4F 53 1C0D 50 6C 61 69 6E 1C0D 1C0D "
#define SAVE " 3C00 3C00 "

TEST( appShowsTheMenu ) {
  Run run;
  run.fs.add( "C:\\CONFIG.SYS", MenuConfig );
  run.fs.add( "C:\\AUTOEXEC.BAT", MenuAutoexec );
  CHECK_INT( run.keys( "" ), 1 );
  CHECK( run.end.find( "Boot Magic - boot menu of drive C:" ) == 0 );
  int row = run.end.find( "NORMAL" );
  CHECK( row > 0 && strstr( run.end.rows[row], "Normal start" ) && strstr( run.end.rows[row], "1 line" ) );
  CHECK( strstr( run.end.rows[row + 1], "GAMES" ) && strstr( run.end.rows[row + 1], "2 lines" ) );
  CHECK( run.end.find( "Default: NORMAL after 10 s" ) > 0 );
  CHECK( run.end.find( "MSDOS.SYS" ) > 0 );
}

TEST( appMakesTheFirstItemAndSaves ) {
  Run run;
  run.fs.add( "C:\\CONFIG.SYS", PlainConfig );
  run.fs.add( "C:\\AUTOEXEC.BAT", PlainAutoexec );
  CHECK_INT( run.keys( NEW_DOS_ITEM SAVE "011B" ), 0 );
  CHECK_STR( run.fs.data( "C:\\CONFIG.SYS" ), "[MENU]\r\nMENUITEM=DOS,Plain\r\n\r\n[COMMON]\r\n"
                                              "DEVICE=C:\\DOS\\HIMEM.SYS\r\nDOS=HIGH\r\nFILES=30\r\n\r\n[DOS]\r\n" );
  CHECK( strstr( run.fs.data( "C:\\AUTOEXEC.BAT" ), "GOTO %CONFIG%\r\n:DOS\r\nGOTO END\r\n:END\r\n" ) );
  CHECK_STR( run.fs.data( "C:\\APPS\\BMAGIC\\BACKUP\\001\\CONFIG.SYS" ), PlainConfig );
  CHECK( strstr( run.out.text, "restart" ) );
}

TEST( appReviewShowsTheDiff ) {
  Run run;
  run.fs.add( "C:\\CONFIG.SYS", PlainConfig );
  run.fs.add( "C:\\AUTOEXEC.BAT", PlainAutoexec );
  CHECK_INT( run.keys( NEW_DOS_ITEM "3C00" ), 1 );
  CHECK( run.end.find( "Review the changes" ) == 0 );
  CHECK( run.end.find( "CONFIG.SYS 1/2" ) == 0 );
  CHECK( run.end.find( "removed: 0   added: 6" ) == 1 );
  int row = run.end.find( "+ [MENU]" );
  CHECK( row > 1 && strstr( run.end.rows[row], "   1" ) );
  CHECK( run.end.find( "  DEVICE=C:\\DOS\\HIMEM.SYS" ) > row );
  CHECK_STR( run.fs.data( "C:\\CONFIG.SYS" ), PlainConfig );
}

TEST( appEditsALineOfAnItem ) {
  Run run;
  run.fs.add( "C:\\CONFIG.SYS", MenuConfig );
  run.fs.add( "C:\\AUTOEXEC.BAT", MenuAutoexec );
  /* GAMES, Enter, the first line, Enter, End, " NOEMS" instead of " RAM", Enter */
  run.keys( "5000 1C0D 5000 5000 1C0D 4F00 0E08 0E08 0E08 4E 4F 45 4D 53 1C0D 011B" SAVE "011B" );
  CHECK( strstr( run.fs.data( "C:\\CONFIG.SYS" ), "[GAMES]\r\nDEVICE=C:\\DOS\\HIMEM.SYS\r\n"
                                                  "DEVICE=C:\\DOS\\EMM386.EXE NOEMS\r\n" ) );
}

TEST( appAddsAndDeletesLines ) {
  Run run;
  run.fs.add( "C:\\CONFIG.SYS", MenuConfig );
  run.fs.add( "C:\\AUTOEXEC.BAT", MenuAutoexec );
  /* NORMAL: Ins on the header adds "X=1" at the top; the :NORMAL header, Down, Del deletes SMARTDRV */
  CHECK_INT( run.keys( "1C0D 5200 58 3D 31 1C0D 5000 5000 5000 5000 5300 011B" SAVE "011B" ), 0 );
  CHECK( strstr( run.fs.data( "C:\\CONFIG.SYS" ), "[NORMAL]\r\nX=1\r\nDEVICE=C:\\DOS\\HIMEM.SYS\r\n" ) );
  CHECK( strstr( run.fs.data( "C:\\AUTOEXEC.BAT" ), ":NORMAL\r\nGOTO END\r\n" ) );
}

TEST( appDeletesAnItemAfterYes ) {
  Run run;
  run.fs.add( "C:\\CONFIG.SYS", MenuConfig );
  run.fs.add( "C:\\AUTOEXEC.BAT", MenuAutoexec );
  CHECK_INT( run.keys( "5000 5300 6E 5300 79" SAVE "011B" ), 0 );
  CHECK( !strstr( run.fs.data( "C:\\CONFIG.SYS" ), "GAMES" ) );
  CHECK( !strstr( run.fs.data( "C:\\AUTOEXEC.BAT" ), "GAMES" ) );
}

TEST( appQuitsWithoutSavingAfterNo ) {
  Run run;
  run.fs.add( "C:\\CONFIG.SYS", MenuConfig );
  run.fs.add( "C:\\AUTOEXEC.BAT", MenuAutoexec );
  CHECK_INT( run.keys( "5000 5300 79 011B 6E" ), 1 );
  CHECK_STR( run.fs.data( "C:\\CONFIG.SYS" ), MenuConfig );
  CHECK( !run.fs.exists( "C:\\APPS\\BMAGIC\\BACKUP" ) );
}

TEST( appSetsTheMenuOptions ) {
  Run run;
  run.fs.add( "C:\\CONFIG.SYS", MenuConfig );
  run.fs.add( "C:\\AUTOEXEC.BAT", MenuAutoexec );
  /* F6: Default GAMES, Timeout 5, Color 15,1, NumLock OFF */
  run.keys( "4000 1519 47 41 4D 45 53 0F09 1519 35 0F09 31 35 2C 31 0F09 4F 46 46 1C0D" SAVE "011B" );
  CHECK( strstr( run.fs.data( "C:\\CONFIG.SYS" ), "MENUDEFAULT=GAMES,5\r\nMENUCOLOR=15,1\r\nNUMLOCK=OFF\r\n" ) );
}

TEST( appRejectsAWrongName ) {
  Run run;
  run.fs.add( "C:\\CONFIG.SYS", MenuConfig );
  run.fs.add( "C:\\AUTOEXEC.BAT", MenuAutoexec );
  run.keys( "5200 47 41 4D 45 53 1C0D 1C0D 1C0D" );
  CHECK( run.end.find( "That name is taken" ) > 0 );
}

TEST( appSetsTheWindowsStartup ) {
  Run run;
  run.fs.add( "C:\\CONFIG.SYS", MenuConfig );
  run.fs.add( "C:\\AUTOEXEC.BAT", MenuAutoexec );
  run.fs.add( "C:\\MSDOS.SYS", "[Paths]\r\nWinDir=C:\\WINDOWS\r\n[Options]\r\nBootGUI=1\r\n" );
  /* F7: BootMenu 1, then BootGUI 0 */
  run.keys( "4100 31 0F09 0F09 0F09 1519 30 3C00" SAVE "011B" );
  CHECK_STR( run.fs.data( "C:\\MSDOS.SYS" ),
             "[Paths]\r\nWinDir=C:\\WINDOWS\r\n[Options]\r\nBootGUI=0\r\nBootMenu=1\r\n" );
}

TEST( appRestoresABackup ) {
  Run run;
  run.fs.add( "C:\\CONFIG.SYS", PlainConfig );
  run.fs.add( "C:\\AUTOEXEC.BAT", PlainAutoexec );
  /* make a menu and save it (backup 001), then F9, Down to 001, R, Y */
  CHECK_INT( run.keys( NEW_DOS_ITEM SAVE "4300 5000 72 79 011B 011B" ), 0 );
  CHECK_STR( run.fs.data( "C:\\CONFIG.SYS" ), PlainConfig );
  CHECK( run.fs.has( "C:\\APPS\\BMAGIC\\BACKUP\\002\\CONFIG.SYS" ) );
}

TEST( appComparesABackup ) {
  Run run;
  run.fs.add( "C:\\CONFIG.SYS", PlainConfig );
  run.fs.add( "C:\\AUTOEXEC.BAT", PlainAutoexec );
  run.keys( NEW_DOS_ITEM SAVE "4300 1C0D" );
  CHECK( run.end.find( "Backup 001 -> the files now on the disk" ) == 0 );
  CHECK( run.end.find( "+ [MENU]" ) > 0 );
}

TEST( appTakesADrive ) {
  Run run;
  run.fs.add( "D:\\CONFIG.SYS", MenuConfig );
  CHECK_INT( run.keys( "", "D:" ), 1 );
  CHECK( run.end.find( "drive D:" ) == 0 );
  CHECK( run.end.find( "NORMAL" ) > 0 );
  Run bad;
  CHECK_INT( bad.keys( "", "/X" ), 2 );
  CHECK( strstr( bad.out.text, "Usage" ) );
}

TEST( appMovesItemsAndLines ) {
  Run run;
  run.fs.add( "C:\\CONFIG.SYS", MenuConfig );
  run.fs.add( "C:\\AUTOEXEC.BAT", MenuAutoexec );
  /* GAMES up (Ctrl+PgUp); then its lines: the EMM386 line up (Ctrl+PgUp) */
  CHECK_INT( run.keys( "5000 8400 1C0D 5000 5000 8400 011B" SAVE "011B" ), 0 );
  CHECK(
      strstr( run.fs.data( "C:\\CONFIG.SYS" ), "MENUITEM=GAMES,Games with EMS\r\nMENUITEM=NORMAL,Normal start\r\n" ) );
  CHECK( strstr( run.fs.data( "C:\\CONFIG.SYS" ),
                 "[GAMES]\r\nDEVICE=C:\\DOS\\EMM386.EXE RAM\r\nDEVICE=C:\\DOS\\HIMEM.SYS\r\n" ) );
}

TEST( appWarnsBeforeARestoreDropsEdits ) {
  Run run;
  run.fs.add( "C:\\CONFIG.SYS", MenuConfig );
  run.fs.add( "C:\\AUTOEXEC.BAT", MenuAutoexec );
  run.keys( "4300 5200 011B 5300 79 4300 72" );
  CHECK( run.end.find( "The edits not saved yet are dropped. Restore backup 001? (Y/N)" ) == 23 );
}
