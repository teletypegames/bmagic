#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "error.h"

Error::Error() {
  text_[0] = 0;
}

void Error::set( const char *fmt, ... ) {
  va_list ap;
  va_start( ap, fmt );
  vsnprintf( text_, sizeof( text_ ), fmt, ap );
  va_end( ap );
}

void Error::clear() {
  text_[0] = 0;
}

bool Error::isSet() const {
  return text_[0] != 0;
}

const char *Error::text() const {
  return text_;
}

ErrorKeeper::ErrorKeeper( Error &error ) : error_( error ) {
  strcpy( kept_, error.text() );
}

ErrorKeeper::~ErrorKeeper() {
  error_.set( "%s", kept_ );
}
