#include <string.h>

#include "byte_sink.h"

MemorySink::MemorySink( char *buf, unsigned size ) : buf_( buf ), size_( size ), len_( 0 ) {
  buf_[0] = 0;
}

bool MemorySink::write( const char *buf, unsigned len, Error & ) {
  unsigned room = size_ - 1 - len_;
  if ( len > room ) len = room;
  memcpy( buf_ + len_, buf, len );
  len_ += len;
  buf_[len_] = 0;
  return true;
}
