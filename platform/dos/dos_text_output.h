#ifndef DOSLIB_PLATFORM_DOS_TEXT_OUTPUT_H
#define DOSLIB_PLATFORM_DOS_TEXT_OUTPUT_H

#include "../text_output.h"

/* The standard output handle, without the stdio library (a smaller program). */
class DosTextOutput : public TextOutput {
public:
  void write( const char *s );
};

#endif
