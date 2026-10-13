#include "screen_dump.h"
#include "../platform/file_system.h"
#include "../platform/screen.h"

bool dumpScreen( Screen &screen, FileSystem &fs, const char *path ) {
  unsigned char row[Screen::Cols * 2];
  int h = fs.create( path );
  if ( h < 0 ) return false;
  bool ok = true;
  for ( int y = 0; y < Screen::Rows; y++ ) {
    for ( int x = 0; x < Screen::Cols; x++ ) {
      unsigned cell = screen.cell( y * Screen::Cols + x );
      row[2 * x] = (unsigned char)cell;
      row[2 * x + 1] = (unsigned char)( cell >> 8 );
    }
    if ( fs.write( h, row, sizeof( row ) ) != (int)sizeof( row ) ) ok = false;
  }
  return fs.close( h ) && ok;
}
