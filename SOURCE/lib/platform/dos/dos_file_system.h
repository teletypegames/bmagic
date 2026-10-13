/*
   Files through the DOS-style C library of Open Watcom (_dos_findfirst, open, ...). The
   Win32 C library has the same calls, so this serves both builds.
*/

#ifndef DOSLIB_PLATFORM_DOS_FILE_SYSTEM_H
#define DOSLIB_PLATFORM_DOS_FILE_SYSTEM_H

#include "../file_system.h"

class DosFileSystem : public FileSystem {
public:
  void list( const char *pattern, unsigned attribs, DirVisitor &visitor );
  bool find( const char *path, unsigned attribs, DirEntry &entry );
  bool exists( const char *path );
  int openRead( const char *path );
  int create( const char *path );
  int read( int handle, void *buf, unsigned size );
  int write( int handle, const void *buf, unsigned len );
  bool close( int handle );
  bool remove( const char *path );
  bool rename( const char *from, const char *to );
  bool makeFolder( const char *path );
  bool removeFolder( const char *path );
  void makeWritable( const char *path );
};

#endif
