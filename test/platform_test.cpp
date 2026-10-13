#include "../core/error.h"
#include "../platform/file_system.h"
#include "../testing/host/fake_file_system.h"
#include "../testing/host/memory_screen.h"
#include "../testing/host/minitest.h"
#include "../testing/scripted_keyboard.h"
#include "../ui/canvas.h"
#include "../ui/list_cursor.h"

TEST( makeFoldersCreatesTheParents ) {
  FakeFileSystem fs;
  CHECK( makeFolders( fs, "C:\\A\\B\\C" ) );
  CHECK( isFolder( fs, "C:\\A\\B" ) );
  CHECK( isFolder( fs, "C:\\A\\B\\C" ) );
}

TEST( removeTreeDeletesEverything ) {
  FakeFileSystem fs;
  Error error;
  fs.add( "C:\\APPS\\X\\ONE.TXT", "1" );
  fs.add( "C:\\APPS\\X\\SUB\\TWO.TXT", "2" );
  CHECK( removeTree( fs, "C:\\APPS\\X", error ) );
  CHECK( !fs.exists( "C:\\APPS\\X" ) );
  CHECK( fs.exists( "C:\\APPS" ) );
}

TEST( removeTreeTellsWhatStayed ) {
  FakeFileSystem fs;
  Error error;
  fs.add( "C:\\APPS\\X\\RUN.EXE", "1" );
  fs.lock( "C:\\APPS\\X\\RUN.EXE" );
  CHECK( !removeTree( fs, "C:\\APPS\\X", error ) );
  CHECK_STR( error.text(), "Cannot remove C:\\APPS\\X" );
}

TEST( readTextTerminates ) {
  FakeFileSystem fs;
  char buf[4];
  fs.add( "C:\\F.TXT", "abcdef" );
  CHECK_INT( readText( fs, "C:\\F.TXT", buf, sizeof( buf ) ), 3 );
  CHECK_STR( buf, "abc" );
  CHECK_INT( readText( fs, "C:\\NONE.TXT", buf, sizeof( buf ) ), -1 );
}

TEST( canvasFieldPadsAndCuts ) {
  MemoryScreen screen;
  Canvas canvas( screen );
  canvas.field( 75, 0, "123456789", 0x1F, 20 );
  CHECK_STR( screen.text( 0 ), "                                                                           12345" );
  canvas.field( 0, 1, "ab", 0x07, 4 );
  CHECK_STR( screen.text( 1 ), "ab" );
  CHECK_INT( screen.attr( 1, 3 ), 0x07 );
}

TEST( canvasFooterTogglesTheHighlight ) {
  MemoryScreen screen;
  Canvas canvas( screen );
  canvas.footer( 0, "~F1~ Help" );
  CHECK_STR( screen.text( 0 ), "F1 Help" );
  CHECK_INT( screen.attr( 0, 0 ), 0x74 );
  CHECK_INT( screen.attr( 0, 3 ), 0x70 );
}

TEST( listCursorKeepsTheSelectionInView ) {
  ListCursor cursor;
  cursor.setRows( 5 );
  cursor.setCount( 20 );
  cursor.select( 12 );
  CHECK_INT( cursor.top(), 8 );
  cursor.select( 3 );
  CHECK_INT( cursor.top(), 3 );
  cursor.setCount( 2 );
  cursor.clamp();
  CHECK_INT( cursor.sel(), 1 );
  cursor.setCount( 0 );
  cursor.clamp();
  CHECK_INT( cursor.sel(), 0 );
}

class EndWithEsc : public ScriptEnd {
public:
  EndWithEsc() : ended( 0 ) {}
  unsigned scriptEnded() {
    ended++;
    return 0x011B;
  }
  int ended;
};

TEST( scriptedKeyboardEndsAtZero ) {
  EndWithEsc end;
  ScriptedKeyboard keys( " 5000\r\n1C0D 0 4800", end );
  CHECK_INT( keys.read(), 0x5000 );
  CHECK_INT( keys.read(), 0x1C0D );
  CHECK_INT( keys.read(), 0x011B );
  CHECK_INT( end.ended, 1 );
}
