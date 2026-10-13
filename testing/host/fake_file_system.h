/*
   An in-memory FileSystem for the unit tests. Paths are DOS paths ("C:\APPS\X.EXE"),
   compared case-insensitively. Folders exist when they were made, or when a file was
   added under them with add().
*/

#ifndef DOSLIB_TESTING_FAKE_FILE_SYSTEM_H
#define DOSLIB_TESTING_FAKE_FILE_SYSTEM_H

#include "../../platform/file_system.h"

class FakeFileSystem : public FileSystem {
public:
  enum { MaxFiles = 128, MaxFolders = 64, MaxHandles = 8, PathSize = 128 };

  FakeFileSystem();
  ~FakeFileSystem();
  /* A file with its parent folders. */
  void add( const char *path, const char *data, unsigned date = 0x5B4A, unsigned time = 0x6000 );
  void addBytes( const char *path, const void *data, unsigned len, unsigned date = 0x5B4A, unsigned time = 0x6000 );
  bool has( const char *path ) const;
  /* The file's data, terminated (empty when it is missing). */
  const char *data( const char *path ) const;
  unsigned size( const char *path ) const;
  /* A file that cannot be replaced or deleted (a running executable on Windows). */
  void lock( const char *path );
  int openCount() const;

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

private:
  struct File {
    char path[PathSize];
    char *data;
    unsigned size;
    unsigned date, time;
    bool used, locked;
  };
  struct Handle {
    int file;
    unsigned pos;
    bool used, writing;
  };

  static void keyOf( const char *path, char *out );
  static void parentOf( const char *path, char *out );
  int fileIndex( const char *key ) const;
  int folderIndex( const char *key ) const;
  bool parentExists( const char *key ) const;
  bool hasChildren( const char *key ) const;
  int allocHandle( int fileIndex, bool writing );
  void addFolders( const char *key );
  int newFile( const char *key );
  void setData( File &f, const void *data, unsigned len );
  void fillEntry( const char *key, DirEntry &entry ) const;

  File files_[MaxFiles];
  char folders_[MaxFolders][PathSize];
  int folderCount_;
  Handle handles_[MaxHandles];
};

#endif
