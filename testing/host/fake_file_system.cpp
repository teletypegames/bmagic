#include <stdlib.h>
#include <string.h>

#include "fake_file_system.h"

FakeFileSystem::FakeFileSystem() : folderCount_( 0 ) {
  memset( files_, 0, sizeof( files_ ) );
  memset( handles_, 0, sizeof( handles_ ) );
}

FakeFileSystem::~FakeFileSystem() {
  for ( int i = 0; i < MaxFiles; i++ ) free( files_[i].data );
}

void FakeFileSystem::keyOf( const char *path, char *out ) {
  int n = 0;
  for ( const char *p = path; *p && n < PathSize - 1; p++ ) {
    char c = *p == '/' ? '\\' : *p;
    out[n++] = ( c >= 'a' && c <= 'z' ) ? (char)( c - 32 ) : c;
  }
  while ( n > 3 && out[n - 1] == '\\' ) n--;
  out[n] = 0;
}

void FakeFileSystem::parentOf( const char *path, char *out ) {
  strcpy( out, path );
  char *slash = strrchr( out, '\\' );
  if ( slash == NULL ) out[0] = 0;
  else if ( slash == out + 2 && out[1] == ':' ) slash[1] = 0;
  else *slash = 0;
}

int FakeFileSystem::fileIndex( const char *key ) const {
  for ( int i = 0; i < MaxFiles; i++ ) {
    if ( files_[i].used && strcmp( files_[i].path, key ) == 0 ) return i;
  }
  return -1;
}

int FakeFileSystem::folderIndex( const char *key ) const {
  for ( int i = 0; i < folderCount_; i++ ) {
    if ( strcmp( folders_[i], key ) == 0 ) return i;
  }
  return -1;
}

bool FakeFileSystem::parentExists( const char *key ) const {
  char parent[PathSize];
  parentOf( key, parent );
  return strlen( parent ) <= 3 || folderIndex( parent ) >= 0;
}

void FakeFileSystem::addFolders( const char *key ) {
  char parent[PathSize];
  parentOf( key, parent );
  if ( strlen( parent ) <= 3 || folderIndex( parent ) >= 0 ) return;
  addFolders( parent );
  strcpy( folders_[folderCount_++], parent );
}

int FakeFileSystem::newFile( const char *key ) {
  int i = fileIndex( key );
  if ( i < 0 ) {
    for ( i = 0; i < MaxFiles && files_[i].used; i++ );
    if ( i == MaxFiles ) return -1;
    memset( &files_[i], 0, sizeof( files_[i] ) );
    strcpy( files_[i].path, key );
    files_[i].used = true;
  }
  files_[i].date = 0x5B4A;
  files_[i].time = 0x6000;
  setData( files_[i], "", 0 );
  return i;
}

void FakeFileSystem::setData( File &f, const void *data, unsigned len ) {
  free( f.data );
  f.data = (char *)malloc( len + 1 );
  memcpy( f.data, data, len );
  f.data[len] = 0;
  f.size = len;
}

void FakeFileSystem::addBytes( const char *path, const void *data, unsigned len, unsigned date, unsigned time ) {
  char key[PathSize];
  keyOf( path, key );
  addFolders( key );
  int i = newFile( key );
  setData( files_[i], data, len );
  files_[i].date = date;
  files_[i].time = time;
}

void FakeFileSystem::add( const char *path, const char *data, unsigned date, unsigned time ) {
  addBytes( path, data, strlen( data ), date, time );
}

bool FakeFileSystem::has( const char *path ) const {
  char key[PathSize];
  keyOf( path, key );
  return fileIndex( key ) >= 0;
}

const char *FakeFileSystem::data( const char *path ) const {
  char key[PathSize];
  keyOf( path, key );
  int i = fileIndex( key );
  return i < 0 ? "" : files_[i].data;
}

unsigned FakeFileSystem::size( const char *path ) const {
  char key[PathSize];
  keyOf( path, key );
  int i = fileIndex( key );
  return i < 0 ? 0 : files_[i].size;
}

void FakeFileSystem::lock( const char *path ) {
  char key[PathSize];
  keyOf( path, key );
  int i = fileIndex( key );
  if ( i >= 0 ) files_[i].locked = true;
}

int FakeFileSystem::openCount() const {
  int n = 0;
  for ( int i = 0; i < MaxHandles; i++ ) n += handles_[i].used;
  return n;
}

static bool nameMatches( const char *name, const char *mask ) {
  return strcmp( mask, "*.*" ) == 0 || strcmp( mask, "*" ) == 0 || strcmp( name, mask ) == 0;
}

static int compareNames( const void *a, const void *b ) {
  return strcmp( ( (const DirEntry *)a )->name, ( (const DirEntry *)b )->name );
}

void FakeFileSystem::fillEntry( const char *key, DirEntry &entry ) const {
  memset( &entry, 0, sizeof( entry ) );
  const char *slash = strrchr( key, '\\' );
  strncpy( entry.name, slash ? slash + 1 : key, sizeof( entry.name ) - 1 );
  int i = fileIndex( key );
  if ( i >= 0 ) {
    entry.date = files_[i].date;
    entry.time = files_[i].time;
    entry.size = files_[i].size;
  }
  else entry.attrib = AttrFolder;
}

void FakeFileSystem::list( const char *pattern, unsigned attribs, DirVisitor &visitor ) {
  char key[PathSize], dir[PathSize], parent[PathSize];
  DirEntry entries[MaxFiles + MaxFolders];
  int count = 0;
  keyOf( pattern, key );
  parentOf( key, dir );
  const char *mask = strrchr( key, '\\' ) + 1;
  for ( int i = 0; i < MaxFiles; i++ ) {
    if ( !files_[i].used ) continue;
    parentOf( files_[i].path, parent );
    fillEntry( files_[i].path, entries[count] );
    if ( strcmp( parent, dir ) == 0 && nameMatches( entries[count].name, mask ) ) count++;
  }
  for ( int i = 0; ( attribs & AttrFolder ) && i < folderCount_; i++ ) {
    parentOf( folders_[i], parent );
    fillEntry( folders_[i], entries[count] );
    if ( strcmp( parent, dir ) == 0 && nameMatches( entries[count].name, mask ) ) count++;
  }
  qsort( entries, count, sizeof( entries[0] ), compareNames );
  for ( int i = 0; i < count; i++ ) {
    if ( !visitor.visit( entries[i] ) ) break;
  }
}

bool FakeFileSystem::find( const char *path, unsigned attribs, DirEntry &entry ) {
  char key[PathSize];
  keyOf( path, key );
  if ( fileIndex( key ) < 0 && !( ( attribs & AttrFolder ) && folderIndex( key ) >= 0 ) ) return false;
  fillEntry( key, entry );
  return true;
}

bool FakeFileSystem::exists( const char *path ) {
  char key[PathSize];
  keyOf( path, key );
  return fileIndex( key ) >= 0 || folderIndex( key ) >= 0;
}

int FakeFileSystem::allocHandle( int fileIdx, bool writing ) {
  for ( int h = 0; h < MaxHandles; h++ ) {
    if ( handles_[h].used ) continue;
    Handle open = { fileIdx, 0, true, writing };
    handles_[h] = open;
    return h;
  }
  return -1;
}

int FakeFileSystem::openRead( const char *path ) {
  char key[PathSize];
  keyOf( path, key );
  int f = fileIndex( key );
  if ( f < 0 ) return -1;
  return allocHandle( f, false );
}

int FakeFileSystem::create( const char *path ) {
  char key[PathSize];
  keyOf( path, key );
  int f = fileIndex( key );
  if ( ( f >= 0 && files_[f].locked ) || !parentExists( key ) ) return -1;
  f = newFile( key );
  return allocHandle( f, true );
}

int FakeFileSystem::read( int handle, void *buf, unsigned size ) {
  if ( handle < 0 || handle >= MaxHandles || !handles_[handle].used ) return -1;
  Handle &h = handles_[handle];
  File &f = files_[h.file];
  unsigned n = f.size - h.pos;
  if ( n > size ) n = size;
  memcpy( buf, f.data + h.pos, n );
  h.pos += n;
  return (int)n;
}

int FakeFileSystem::write( int handle, const void *buf, unsigned len ) {
  if ( handle < 0 || handle >= MaxHandles || !handles_[handle].used || !handles_[handle].writing ) return -1;
  File &f = files_[handles_[handle].file];
  f.data = (char *)realloc( f.data, f.size + len + 1 );
  memcpy( f.data + f.size, buf, len );
  f.size += len;
  f.data[f.size] = 0;
  return (int)len;
}

bool FakeFileSystem::close( int handle ) {
  if ( handle < 0 || handle >= MaxHandles || !handles_[handle].used ) return false;
  handles_[handle].used = false;
  return true;
}

bool FakeFileSystem::remove( const char *path ) {
  char key[PathSize];
  keyOf( path, key );
  int f = fileIndex( key );
  if ( f < 0 || files_[f].locked ) return false;
  free( files_[f].data );
  memset( &files_[f], 0, sizeof( files_[f] ) );
  return true;
}

bool FakeFileSystem::rename( const char *from, const char *to ) {
  char src[PathSize], dst[PathSize];
  keyOf( from, src );
  keyOf( to, dst );
  int f = fileIndex( src );
  if ( f < 0 || fileIndex( dst ) >= 0 || !parentExists( dst ) ) return false;
  strcpy( files_[f].path, dst );
  return true;
}

bool FakeFileSystem::makeFolder( const char *path ) {
  char key[PathSize];
  keyOf( path, key );
  if ( folderIndex( key ) >= 0 || fileIndex( key ) >= 0 || !parentExists( key ) || folderCount_ == MaxFolders )
    return false;
  strcpy( folders_[folderCount_++], key );
  return true;
}

bool FakeFileSystem::hasChildren( const char *key ) const {
  char parent[PathSize];
  for ( int i = 0; i < MaxFiles; i++ ) {
    if ( !files_[i].used ) continue;
    parentOf( files_[i].path, parent );
    if ( strcmp( parent, key ) == 0 ) return true;
  }
  for ( int i = 0; i < folderCount_; i++ ) {
    parentOf( folders_[i], parent );
    if ( strcmp( parent, key ) == 0 ) return true;
  }
  return false;
}

bool FakeFileSystem::removeFolder( const char *path ) {
  char key[PathSize];
  keyOf( path, key );
  int d = folderIndex( key );
  if ( d < 0 || hasChildren( key ) ) return false;
  memmove( folders_[d], folders_[d + 1], ( folderCount_ - d - 1 ) * PathSize );
  folderCount_--;
  return true;
}

void FakeFileSystem::makeWritable( const char * ) {}
