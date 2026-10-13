/*
   Files and folders with DOS path names. The attribute bits are the ones of DOS.
*/

#ifndef DOSLIB_PLATFORM_FILE_SYSTEM_H
#define DOSLIB_PLATFORM_FILE_SYSTEM_H

class Error;

enum { AttrNormal = 0x00, AttrReadOnly = 0x01, AttrHidden = 0x02, AttrSystem = 0x04, AttrFolder = 0x10 };

struct DirEntry {
  char name[13];
  unsigned char attrib;
  unsigned time, date; /* of the last write, DOS format */
  unsigned long size;

  bool isFolder() const { return ( attrib & AttrFolder ) != 0; }
};

/* Gets the entries of a listing; returns false to stop it. */
class DirVisitor {
public:
  virtual bool visit( const DirEntry &entry ) = 0;
};

class FileSystem {
public:
  /* The entries matching pattern ("C:\GAMES\*.*"); attribs selects the hidden, system
     and folder entries besides the normal files, as DOS does. */
  virtual void list( const char *pattern, unsigned attribs, DirVisitor &visitor ) = 0;
  /* The first entry matching path; false when there is none. */
  virtual bool find( const char *path, unsigned attribs, DirEntry &entry ) = 0;
  virtual bool exists( const char *path ) = 0;

  /* A handle >= 0, or -1. */
  virtual int openRead( const char *path ) = 0;
  /* A new, empty file. */
  virtual int create( const char *path ) = 0;
  /* The bytes read or written, -1 on an error. */
  virtual int read( int handle, void *buf, unsigned size ) = 0;
  virtual int write( int handle, const void *buf, unsigned len ) = 0;
  /* false when the data could not be written */
  virtual bool close( int handle ) = 0;

  virtual bool remove( const char *path ) = 0;
  virtual bool rename( const char *from, const char *to ) = 0;
  virtual bool makeFolder( const char *path ) = 0;
  virtual bool removeFolder( const char *path ) = 0;
  /* Clears the read-only, hidden and system attributes. */
  virtual void makeWritable( const char *path ) = 0;
};

/* The whole file (at most size bytes): the bytes read, or -1. */
int readFile( FileSystem &fs, const char *path, void *buf, unsigned size );
/* The same, terminated with a 0 (at most size - 1 bytes). */
int readText( FileSystem &fs, const char *path, char *buf, unsigned size );
/* A new file with the bytes. */
bool writeFile( FileSystem &fs, const char *path, const void *buf, unsigned len );
/* Creates the folder and its missing parents ("C:\A\B" - the drive is not created). */
bool makeFolders( FileSystem &fs, const char *path );
bool isFolder( FileSystem &fs, const char *path );
/* The folder with everything in it; false if anything stayed (error: the last path that
   could not be deleted). In file_tree.cpp. */
bool removeTree( FileSystem &fs, const char *path, Error &error );

#endif
