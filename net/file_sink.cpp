#include "file_sink.h"
#include "../core/error.h"
#include "../platform/file_system.h"

FileSink::FileSink( FileSystem &fs ) : fs_( fs ), handle_( -1 ), size_( 0 ) {}

bool FileSink::open( const char *path ) {
  crc_ = Crc32();
  size_ = 0;
  handle_ = fs_.create( path );
  return handle_ >= 0;
}

bool FileSink::close() {
  if ( handle_ < 0 ) return true;
  bool ok = fs_.close( handle_ );
  handle_ = -1;
  return ok;
}

bool FileSink::write( const char *buf, unsigned len, Error &error ) {
  size_ += len;
  crc_.update( buf, len );
  if ( fs_.write( handle_, buf, len ) != (int)len ) {
    error.set( "Disk full?" );
    return false;
  }
  return true;
}

uint32_t FileSink::crc() const {
  return crc_.value();
}

uint32_t FileSink::size() const {
  return size_;
}
