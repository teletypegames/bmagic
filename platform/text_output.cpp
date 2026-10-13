#include <stdarg.h>
#include <stdio.h>

#include "text_output.h"

void printTo( TextOutput &out, const char *fmt, ... ) {
  char buf[256];
  va_list ap;
  va_start( ap, fmt );
  vsnprintf( buf, sizeof( buf ), fmt, ap );
  va_end( ap );
  out.write( buf );
}
