#include <string.h>

#include "name_list.h"
#include "str.h"

NameList::NameList( char *buf, unsigned size ) : buf_( buf ), size_( size ), used_( 0 ), count_( 0 ) {
  buf_[0] = 0;
}

char *NameList::buffer() {
  return buf_;
}

unsigned NameList::size() const {
  return size_;
}

void NameList::setCount( int count ) {
  count_ = count;
  const char *p = buf_;
  for ( int i = 0; i < count; i++ ) p += strlen( p ) + 1;
  used_ = p - buf_;
}

int NameList::count() const {
  return count_;
}

bool NameList::add( const char *name ) {
  unsigned len = strlen( name ) + 1;
  if ( used_ + len + 1 > size_ ) return false;
  memcpy( buf_ + used_, name, len );
  used_ += len;
  buf_[used_] = 0;
  count_++;
  return true;
}

bool NameList::contains( const char *name ) const {
  for ( const char *n = first(); n; n = next( n ) ) {
    if ( cmpi( n, name ) == 0 ) return true;
  }
  return false;
}

void NameList::clear() {
  used_ = 0;
  count_ = 0;
  buf_[0] = 0;
}

const char *NameList::first() const {
  return count_ ? buf_ : NULL;
}

const char *NameList::next( const char *name ) const {
  const char *n = name + strlen( name ) + 1;
  return n < buf_ + used_ ? n : NULL;
}
