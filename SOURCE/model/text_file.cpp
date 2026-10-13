#include <string.h>

#include "text_file.h"
#include "../lib/core/sort.h"

TextFile::TextFile() {
  clear();
}

void TextFile::clear() {
  count_ = 0;
  used_ = 0;
  lfOnly_ = false;
  lastEnds_ = true;
  eof_ = false;
}

bool TextFile::isBinary( const char *text, unsigned len ) {
  return memchr( text, 0, len ) != NULL;
}

bool TextFile::load( const char *text, unsigned len ) {
  clear();
  const char *end = text + len;
  const char *eof = (const char *)memchr( text, 0x1A, len );
  if ( eof ) {
    end = eof;
    eof_ = true;
  }
  bool cr = false, lf = false;
  const char *p = text;
  while ( p < end ) {
    const char *s = p;
    while ( p < end && *p != '\r' && *p != '\n' ) p++;
    unsigned n = p - s;
    if ( count_ == MaxLines || used_ + n + 1 > PoolSize ) return false;
    start_[count_++] = (unsigned short)used_;
    memcpy( pool_ + used_, s, n );
    used_ += n;
    pool_[used_++] = 0;
    lastEnds_ = p < end;
    if ( p < end && *p == '\r' ) {
      cr = true;
      p++;
    }
    if ( p < end && *p == '\n' ) {
      lf = true;
      p++;
    }
  }
  if ( count_ == 0 ) lastEnds_ = true;
  lfOnly_ = lf && !cr;
  return true;
}

class ByStart {
public:
  explicit ByStart( const unsigned short *start ) : start_( start ) {}
  int compare( unsigned short a, unsigned short b ) { return start_[a] - start_[b]; }

private:
  const unsigned short *start_;
};

void TextFile::compact() {
  for ( int i = 0; i < count_; i++ ) order_[i] = (unsigned short)i;
  ByStart byStart( start_ );
  shellSortBy( order_, count_, byStart );
  unsigned w = 0;
  for ( int k = 0; k < count_; k++ ) {
    int i = order_[k];
    unsigned n = strlen( pool_ + start_[i] ) + 1;
    memmove( pool_ + w, pool_ + start_[i], n );
    start_[i] = (unsigned short)w;
    w += n;
  }
  used_ = w;
}

bool TextFile::store( const char *text, unsigned short &at ) {
  unsigned n = strlen( text ) + 1;
  if ( used_ + n > PoolSize ) compact();
  if ( used_ + n > PoolSize ) return false;
  memcpy( pool_ + used_, text, n );
  at = (unsigned short)used_;
  used_ += n;
  return true;
}

bool TextFile::insert( int at, const char *text ) {
  if ( count_ == MaxLines || at < 0 || at > count_ ) return false;
  unsigned short pos;
  if ( !store( text, pos ) ) return false;
  memmove( start_ + at + 1, start_ + at, ( count_ - at ) * sizeof( start_[0] ) );
  start_[at] = pos;
  count_++;
  return true;
}

bool TextFile::duplicate( int at, int i ) {
  if ( count_ == MaxLines || at < 0 || at > count_ ) return false;
  unsigned n = strlen( line( i ) ) + 1;
  if ( used_ + n > PoolSize ) compact();
  if ( used_ + n > PoolSize ) return false;
  memcpy( pool_ + used_, line( i ), n );
  memmove( start_ + at + 1, start_ + at, ( count_ - at ) * sizeof( start_[0] ) );
  start_[at] = (unsigned short)used_;
  used_ += n;
  count_++;
  return true;
}

bool TextFile::replace( int i, const char *text ) {
  char *old = pool_ + start_[i];
  if ( strlen( text ) <= strlen( old ) ) {
    memmove( old, text, strlen( text ) + 1 );
    return true;
  }
  unsigned short pos;
  if ( !store( text, pos ) ) return false;
  start_[i] = pos;
  return true;
}

void TextFile::remove( int i ) {
  memmove( start_ + i, start_ + i + 1, ( count_ - i - 1 ) * sizeof( start_[0] ) );
  count_--;
}

void TextFile::swap( int i, int j ) {
  unsigned short t = start_[i];
  start_[i] = start_[j];
  start_[j] = t;
}

bool TextFile::sameAs( const TextFile &other ) const {
  if ( count_ != other.count_ || lfOnly_ != other.lfOnly_ || lastEnds_ != other.lastEnds_ || eof_ != other.eof_ )
    return false;
  for ( int i = 0; i < count_; i++ ) {
    if ( strcmp( line( i ), other.line( i ) ) ) return false;
  }
  return true;
}

unsigned TextFile::write( char *out ) const {
  char *d = out;
  for ( int i = 0; i < count_; i++ ) {
    unsigned n = strlen( line( i ) );
    memcpy( d, line( i ), n );
    d += n;
    if ( i == count_ - 1 && !lastEnds_ ) break;
    if ( !lfOnly_ ) *d++ = '\r';
    *d++ = '\n';
  }
  if ( eof_ ) *d++ = 0x1A;
  return d - out;
}
