#include <string.h>

#include "str.h"

char upcase( char c ) {
  return ( c >= 'a' && c <= 'z' ) ? (char)( c - 32 ) : c;
}

void upcaseStr( char *s ) {
  for ( ; *s; s++ ) *s = upcase( *s );
}

int cmpi( const char *a, const char *b ) {
  for ( ;; ) {
    unsigned char x = (unsigned char)upcase( *a++ );
    unsigned char y = (unsigned char)upcase( *b++ );
    if ( x != y ) return x > y ? 1 : -1;
    if ( x == 0 ) return 0;
  }
}

void copyStr( char *dst, const char *src, int size ) {
  strncpy( dst, src, size - 1 );
  dst[size - 1] = 0;
}

static bool isSpace( char c ) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

char *trim( char *s ) {
  while ( *s == ' ' || *s == '\t' ) s++;
  char *e = s + strlen( s );
  while ( e > s && isSpace( e[-1] ) ) e--;
  *e = 0;
  return s;
}

const char *skipBlank( const char *s ) {
  while ( *s == ' ' || *s == '\t' ) s++;
  return s;
}

const char *trimBack( const char *beg, const char *end ) {
  while ( end != beg && ( end[-1] == ' ' || end[-1] == '\t' ) ) end--;
  return end;
}

bool isEol( char c ) {
  return c == 0 || c == '\r' || c == '\n' || c == 0x1A;
}

char *numStr( char *d, unsigned n ) {
  char digits[6];
  int i = 0;
  do {
    digits[i++] = (char)( '0' + n % 10 );
    n /= 10;
  } while ( n );
  while ( i ) *d++ = digits[--i];
  *d = 0;
  return d;
}

void joinPath( char *out, const char *a, const char *b, char sep ) {
  if ( out != a ) strcpy( out, a );
  int len = strlen( out );
  if ( len == 0 || out[len - 1] != sep ) {
    out[len++] = sep;
    out[len] = 0;
  }
  strcat( out, b );
}

void joinLocal( char *out, const char *a, const char *b ) {
  joinPath( out, a, b, '\\' );
}

void joinRemote( char *out, const char *a, const char *b ) {
  joinPath( out, a, b, '/' );
}

bool samePath( const char *a, const char *b ) {
  for ( ; *a && *b; a++, b++ ) {
    char x = upcase( *a == '/' ? '\\' : *a );
    char y = upcase( *b == '/' ? '\\' : *b );
    if ( x != y ) return false;
  }
  return *a == *b;
}

bool isDosName( const char *name ) {
  static const char bad[] = " \"*+,/:;<=>?[\\]|";
  int base = 0, ext = 0;
  bool dot = false;
  if ( *name == 0 || *name == '.' ) return false;
  for ( const char *p = name; *p; p++ ) {
    if ( (unsigned char)*p < 32 || strchr( bad, *p ) ) return false;
    if ( *p == '.' ) {
      if ( dot ) return false;
      dot = true;
    }
    else if ( dot ) ext++;
    else base++;
  }
  return base >= 1 && base <= 8 && ext <= 3;
}

bool isExecutable( const char *name ) {
  const char *e = strchr( name, '.' );
  if ( e == NULL || strlen( e ) != 4 ) return false;
  e++;
  return cmpi( e, "COM" ) == 0 || cmpi( e, "EXE" ) == 0 || cmpi( e, "BAT" ) == 0;
}
