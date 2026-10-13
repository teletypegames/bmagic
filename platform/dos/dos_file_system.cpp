#include <direct.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "dos_file_system.h"

static void toEntry( const struct find_t &f, DirEntry &entry ) {
  strncpy( entry.name, f.name, sizeof( entry.name ) - 1 );
  entry.name[sizeof( entry.name ) - 1] = 0;
  entry.attrib = f.attrib;
  entry.time = f.wr_time;
  entry.date = f.wr_date;
  entry.size = f.size;
}

void DosFileSystem::list( const char *pattern, unsigned attribs, DirVisitor &visitor ) {
  struct find_t f;
  if ( _dos_findfirst( pattern, attribs, &f ) ) return;
  DirEntry entry;
  do {
    toEntry( f, entry );
    if ( !visitor.visit( entry ) ) break;
  } while ( _dos_findnext( &f ) == 0 );
  _dos_findclose( &f );
}

bool DosFileSystem::find( const char *path, unsigned attribs, DirEntry &entry ) {
  struct find_t f;
  if ( _dos_findfirst( path, attribs, &f ) ) return false;
  toEntry( f, entry );
  _dos_findclose( &f );
  return true;
}

bool DosFileSystem::exists( const char *path ) {
  return access( path, 0 ) == 0;
}

int DosFileSystem::openRead( const char *path ) {
  return open( path, O_RDONLY | O_BINARY );
}

int DosFileSystem::create( const char *path ) {
  return open( path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE );
}

int DosFileSystem::read( int handle, void *buf, unsigned size ) {
  return ::read( handle, buf, size );
}

int DosFileSystem::write( int handle, const void *buf, unsigned len ) {
  return ::write( handle, buf, len );
}

bool DosFileSystem::close( int handle ) {
  return ::close( handle ) == 0;
}

bool DosFileSystem::remove( const char *path ) {
  return unlink( path ) == 0;
}

bool DosFileSystem::rename( const char *from, const char *to ) {
  return ::rename( from, to ) == 0;
}

bool DosFileSystem::makeFolder( const char *path ) {
  return mkdir( path ) == 0;
}

bool DosFileSystem::removeFolder( const char *path ) {
  return rmdir( path ) == 0;
}

void DosFileSystem::makeWritable( const char *path ) {
  _dos_setfileattr( path, _A_NORMAL );
}
