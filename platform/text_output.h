#ifndef DOSLIB_PLATFORM_TEXT_OUTPUT_H
#define DOSLIB_PLATFORM_TEXT_OUTPUT_H

/* Text on the standard output, written at once. */
class TextOutput {
public:
  virtual void write( const char *s ) = 0;
};

/* printf into out (at most 255 characters). */
void printTo( TextOutput &out, const char *fmt, ... );

#endif
