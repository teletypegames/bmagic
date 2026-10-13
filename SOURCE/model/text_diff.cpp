#include <stddef.h>
#include <string.h>

#include "text_diff.h"

TextDiff::TextDiff() : a_( NULL ), b_( NULL ), count_( 0 ), added_( 0 ), removed_( 0 ) {}

void TextDiff::hash( const TextFile &file, unsigned short *out ) {
  for ( int i = 0; i < file.count(); i++ ) {
    unsigned short h = 0;
    for ( const char *s = file.line( i ); *s; s++ ) h = (unsigned short)( h * 31 + (unsigned char)*s );
    out[i] = h;
  }
}

bool TextDiff::same( int i, int j ) const {
  return hashA_[i] == hashB_[j] && strcmp( a_->line( i ), b_->line( j ) ) == 0;
}

/* i, j agree, and so do the lines after them (or both files end there). */
bool TextDiff::strong( int i, int j ) const {
  if ( !same( i, j ) ) return false;
  int na = a_->count(), nb = b_->count();
  if ( i + 1 == na && j + 1 == nb ) return true;
  return i + 1 < na && j + 1 < nb && same( i + 1, j + 1 );
}

/* The nearest x, y (fewest lines skipped) where the files agree again after i, j. */
bool TextDiff::findSync( int i, int j, bool needStrong, int &x, int &y ) const {
  int na = a_->count(), nb = b_->count();
  for ( int k = 1; k <= 2 * Window; k++ ) {
    for ( x = 0; x <= k; x++ ) {
      y = k - x;
      if ( x > Window || y > Window || i + x >= na || j + y >= nb ) continue;
      if ( needStrong ? strong( i + x, j + y ) : same( i + x, j + y ) ) return true;
    }
  }
  return false;
}

void TextDiff::emit( char kind, int a, int b ) {
  DiffRow &r = rows_[count_++];
  r.kind = kind;
  r.a = a;
  r.b = b;
  if ( kind == DiffAdded ) added_++;
  if ( kind == DiffRemoved ) removed_++;
}

void TextDiff::compare( const TextFile &before, const TextFile &after ) {
  a_ = &before;
  b_ = &after;
  count_ = added_ = removed_ = 0;
  hash( before, hashA_ );
  hash( after, hashB_ );
  int na = before.count(), nb = after.count(), i = 0, j = 0;
  while ( i < na || j < nb ) {
    if ( i < na && j < nb && same( i, j ) ) {
      emit( DiffSame, i++, j++ );
      continue;
    }
    int x = 1, y = 1;
    if ( i == na ) x = 0;
    else if ( j == nb ) y = 0;
    else if ( !findSync( i, j, true, x, y ) && !findSync( i, j, false, x, y ) ) x = y = 1;
    while ( x-- > 0 ) emit( DiffRemoved, i++, -1 );
    while ( y-- > 0 ) emit( DiffAdded, -1, j++ );
  }
  fold();
}

/* The unchanged rows farther than Context from a change become skip rows. */
void TextDiff::fold() {
  int dist = Context + 1;
  for ( int r = count_ - 1; r >= 0; r-- ) {
    dist = rows_[r].kind == DiffSame ? dist + 1 : 0;
    near_[r] = dist <= Context;
  }
  dist = Context + 1;
  for ( int r = 0; r < count_; r++ ) {
    dist = rows_[r].kind == DiffSame ? dist + 1 : 0;
    if ( dist <= Context ) near_[r] = 1;
  }
  int w = 0;
  for ( int r = 0; r < count_; r++ ) {
    if ( rows_[r].kind != DiffSame || near_[r] ) {
      rows_[w++] = rows_[r];
      continue;
    }
    if ( w > 0 && rows_[w - 1].kind == DiffSkip ) rows_[w - 1].b++;
    else {
      rows_[w].kind = DiffSkip;
      rows_[w].a = rows_[r].a;
      rows_[w].b = 1;
      w++;
    }
  }
  count_ = w;
}
