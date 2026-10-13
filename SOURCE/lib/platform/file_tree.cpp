#include <string.h>

#include "file_system.h"
#include "../core/error.h"
#include "../core/str.h"

class TreeRemover : public DirVisitor {
public:
  TreeRemover( FileSystem &fs, const char *path, Error &error )
    : fs_( fs ), path_( path ), error_( error ), ok_( true ) {}
  bool visit( const DirEntry &entry );
  bool ok() const { return ok_; }

private:
  FileSystem &fs_;
  const char *path_;
  Error &error_;
  bool ok_;
};

bool TreeRemover::visit( const DirEntry &entry ) {
  char child[128];
  if ( strcmp( entry.name, "." ) == 0 || strcmp( entry.name, ".." ) == 0 ) return true;
  joinLocal( child, path_, entry.name );
  if ( entry.isFolder() ) {
    if ( !removeTree( fs_, child, error_ ) ) ok_ = false;
  }
  else {
    fs_.makeWritable( child );
    if ( !fs_.remove( child ) ) {
      error_.set( "Cannot delete %s", child );
      ok_ = false;
    }
  }
  return true;
}

bool removeTree( FileSystem &fs, const char *path, Error &error ) {
  char pattern[128];
  TreeRemover remover( fs, path, error );
  joinLocal( pattern, path, "*.*" );
  fs.list( pattern, AttrFolder | AttrHidden | AttrSystem | AttrReadOnly, remover );
  if ( fs.removeFolder( path ) ) return remover.ok();
  error.set( "Cannot remove %s", path );
  return false;
}
