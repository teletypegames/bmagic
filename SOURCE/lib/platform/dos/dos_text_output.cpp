#include <io.h>
#include <string.h>

#include "dos_text_output.h"

void DosTextOutput::write( const char *s ) {
  ::write( 1, s, strlen( s ) );
}
