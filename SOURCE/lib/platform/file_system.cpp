#include <string.h>

#include "file_system.h"
#include "../core/str.h"

int readFile( FileSystem &fs, const char *path, void *buf, unsigned size ) {
  int h = fs.openRead( path );
  if ( h < 0 ) return -1;
  int n = fs.read( h, buf, size );
  fs.close( h );
  return n;
}

int readText( FileSystem &fs, const char *path, char *buf, unsigned size ) {
  int n = readFile( fs, path, buf, size - 1 );
  if ( n < 0 ) return -1;
  buf[n] = 0;
  return n;
}

bool writeFile( FileSystem &fs, const char *path, const void *buf, unsigned len ) {
  int h = fs.create( path );
  if ( h < 0 ) return false;
  bool ok = fs.write( h, buf, len ) == (int)len;
  return fs.close( h ) && ok;
}

bool isFolder( FileSystem &fs, const char *path ) {
  DirEntry entry;
  return fs.find( path, AttrFolder | AttrHidden | AttrSystem, entry ) && entry.isFolder();
}

bool makeFolders( FileSystem &fs, const char *path ) {
  char tmp[128];
  copyStr( tmp, path, sizeof( tmp ) );
  for ( char *p = tmp + 3; *p; p++ ) {
    if ( *p == '\\' ) {
      *p = 0;
      fs.makeFolder( tmp );
      *p = '\\';
    }
  }
  fs.makeFolder( tmp );
  return isFolder( fs, tmp );
}
