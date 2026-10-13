#include <stdio.h>

#include "stdio_text_output.h"

void StdioTextOutput::write( const char *s ) {
  fputs( s, stdout );
  fflush( stdout );
}
