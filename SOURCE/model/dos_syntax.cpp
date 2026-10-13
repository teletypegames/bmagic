#include <stddef.h>
#include <string.h>

#include "dos_syntax.h"
#include "../lib/core/str.h"

static bool isBlank( char c ) {
  return c == ' ' || c == '\t';
}

/* s[0..n) trimmed into out. */
static void copyTrimmed( char *out, int size, const char *s, int n ) {
  while ( n > 0 && isBlank( *s ) ) {
    s++;
    n--;
  }
  while ( n > 0 && isBlank( s[n - 1] ) ) n--;
  if ( n > size - 1 ) n = size - 1;
  memcpy( out, s, n );
  out[n] = 0;
}

bool isBlankLine( const char *line ) {
  return *skipBlank( line ) == 0;
}

bool sectionName( const char *line, char *name, int size ) {
  const char *s = skipBlank( line );
  if ( *s != '[' ) return false;
  const char *e = strchr( s, ']' );
  if ( e == NULL ) return false;
  copyTrimmed( name, size, s + 1, e - s - 1 );
  return true;
}

const char *configValue( const char *line, const char *key ) {
  const char *s = skipBlank( line );
  int n = strlen( key );
  for ( int i = 0; i < n; i++ ) {
    if ( upcase( s[i] ) != key[i] ) return NULL;
  }
  s += n;
  if ( *s != '=' && !isBlank( *s ) ) return NULL;
  s = skipBlank( s );
  if ( *s == '=' ) s = skipBlank( s + 1 );
  return s;
}

bool labelName( const char *line, char *name, int size ) {
  const char *s = skipBlank( line );
  if ( *s != ':' || s[1] == ':' ) return false;
  s = skipBlank( s + 1 );
  int n = 0;
  while ( s[n] && !isBlank( s[n] ) ) n++;
  if ( n == 0 ) return false;
  copyTrimmed( name, size, s, n );
  return true;
}

bool sameLabel( const char *a, const char *b ) {
  for ( int i = 0; i < 8; i++ ) {
    if ( upcase( a[i] ) != upcase( b[i] ) ) return false;
    if ( a[i] == 0 ) return true;
  }
  return true;
}

/* After "GOTO" and its blanks (and a ':'), or NULL. */
static const char *afterGoto( const char *line ) {
  const char *s = skipBlank( line );
  if ( *s == '@' ) s++;
  static const char Goto[] = "GOTO";
  for ( int i = 0; i < 4; i++ ) {
    if ( upcase( s[i] ) != Goto[i] ) return NULL;
  }
  if ( !isBlank( s[4] ) ) return NULL;
  s = skipBlank( s + 4 );
  if ( *s == ':' ) s++;
  return s;
}

bool isGotoConfig( const char *line ) {
  char target[16];
  return gotoTarget( line, target, sizeof( target ) ) && cmpi( target, "%CONFIG%" ) == 0;
}

bool gotoTarget( const char *line, char *name, int size ) {
  const char *s = afterGoto( line );
  if ( s == NULL ) return false;
  int n = 0;
  while ( s[n] && !isBlank( s[n] ) ) n++;
  if ( n == 0 ) return false;
  copyTrimmed( name, size, s, n );
  return true;
}

const char *splitComma( const char *value, char *first, int size ) {
  const char *comma = strchr( value, ',' );
  int n = comma ? comma - value : (int)strlen( value );
  copyTrimmed( first, size, value, n );
  return comma ? comma + 1 : NULL;
}
