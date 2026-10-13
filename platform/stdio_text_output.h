#ifndef DOSLIB_PLATFORM_STDIO_TEXT_OUTPUT_H
#define DOSLIB_PLATFORM_STDIO_TEXT_OUTPUT_H

#include "text_output.h"

class StdioTextOutput : public TextOutput {
public:
  void write( const char *s );
};

#endif
