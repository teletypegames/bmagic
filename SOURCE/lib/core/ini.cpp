#include <string.h>

#include "ini.h"
#include "str.h"
#include "text.h"

bool IniLine::isKeyLine() const {
  return keyBegin != NULL;
}

bool IniLine::keyIs( const char *name ) const {
  if ( keyBegin == NULL ) return false;
  const char *s = keyBegin;
  while ( s != keyEnd && upcase( *s ) == *name ) s++, name++;
  return s == keyEnd && *name == 0;
}

void IniLine::copyValue( char *dst, int size ) const {
  int len = valueLength < size - 1 ? valueLength : size - 1;
  memcpy( dst, value, len );
  dst[len] = 0;
}

IniScanner::IniScanner( const char *text ) : body_( skipBom( text ) ), pos_( body_ ) {}

const char *IniScanner::body() const {
  return body_;
}

bool IniScanner::next( IniLine &line ) {
  const char *s = pos_;
  if ( *s == 0 || *s == 0x1A ) return false;
  const char *e = s;
  while ( !isEol( *e ) ) e++;
  line.begin = s;
  line.end = e;
  line.keyBegin = line.keyEnd = line.value = NULL;
  line.valueLength = 0;
  if ( *e == '\r' ) e++;
  if ( *e == '\n' ) e++;
  pos_ = e;

  s = skipBlank( s );
  if ( *s == ';' || *s == '#' || *s == '[' ) return true;
  const char *key = s;
  while ( s != line.end && *s != '=' ) s++;
  if ( s == line.end ) return true;
  line.keyBegin = key;
  line.keyEnd = trimBack( key, s );
  const char *v = skipBlank( s + 1 );
  const char *ve = trimBack( v, line.end );
  if ( ve - v >= 2 && v[0] == '"' && ve[-1] == '"' ) {
    v++;
    ve--;
  }
  line.value = v;
  line.valueLength = ve - v;
  return true;
}
