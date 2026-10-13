#include <stdarg.h>
#include <stdio.h>

#include "progress.h"

void reportf( Progress &progress, const char *fmt, ... ) {
  char buf[160];
  va_list ap;
  va_start( ap, fmt );
  vsnprintf( buf, sizeof( buf ), fmt, ap );
  va_end( ap );
  progress.report( buf );
}
