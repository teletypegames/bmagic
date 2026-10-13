/*
   BMAGIC.EXE on DOS. The test build (build.sh test) takes its keys from TESTKEYS.TXT in
   the program's folder; when they run out it saves the screen to SCREEN.BIN there and
   exits with code 3.
*/

#include <stdlib.h>
#include <string.h>

#include "app/bmagic.h"
#include "lib/platform/dos/bios_keyboard.h"
#include "lib/platform/dos/dos_file_system.h"
#include "lib/platform/dos/dos_screen.h"
#include "lib/platform/dos/dos_speaker.h"
#include "lib/platform/dos/dos_text_output.h"
#include "lib/testing/scripted_keyboard.h"
#include "lib/testing/screen_dump.h"
#include "platform/dos/dos_boot_system.h"

static char progDir[80];

/* The folder of the program (with a trailing '\'); C:\APPS\BMAGIC\ when argv[0] has none. */
static void locate( const char *argv0 ) {
  const char *slash = strrchr( argv0, '\\' );
  if ( slash == NULL || slash - argv0 + 1 > (int)sizeof( progDir ) - 20 ) {
    strcpy( progDir, "C:\\APPS\\BMAGIC\\" );
    return;
  }
  memcpy( progDir, argv0, slash - argv0 + 1 );
  progDir[slash - argv0 + 1] = 0;
}

static void progFile( const char *name, char *out ) {
  strcpy( out, progDir );
  strcat( out, name );
}

class DumpAndExit : public ScriptEnd {
public:
  DumpAndExit( Screen &screen, FileSystem &fs ) : screen_( screen ), fs_( fs ) {}
  unsigned scriptEnded() {
    char path[96];
    progFile( "SCREEN.BIN", path );
    dumpScreen( screen_, fs_, path );
    exit( 3 );
    return 0;
  }

private:
  Screen &screen_;
  FileSystem &fs_;
};

int main( int argc, char *argv[] ) {
  static DosFileSystem fs;
  static DosScreen screen( DosScreen::SetMode );
  static DosSpeaker speaker;
  static DosTextOutput out;
  static DosBootSystem system;
  locate( argv[0] );
#ifdef TEST
  static char keys[4096];
  char path[96];
  progFile( "TESTKEYS.TXT", path );
  if ( readText( fs, path, keys, sizeof( keys ) ) < 0 ) keys[0] = 0;
  static DumpAndExit end( screen, fs );
  static ScriptedKeyboard keyboard( keys, end );
#else
  static BiosKeyboard keyboard;
#endif
  Machine machine = { fs, screen, keyboard, speaker, out, system };
  return runBmagic( machine, progDir, argc, argv );
}
