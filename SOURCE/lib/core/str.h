/*
   Strings, 8.3 names and paths. Case-insensitive means: only a-z is upper-cased and the
   bytes compare unsigned, so code page 437 letters keep their order.
*/

#ifndef DOSLIB_CORE_STR_H
#define DOSLIB_CORE_STR_H

char upcase( char c );
void upcaseStr( char *s );
int cmpi( const char *a, const char *b );

/* Copies at most size - 1 characters, always terminated. */
void copyStr( char *dst, const char *src, int size );
/* Spaces, tabs, CR and LF off both ends, in place: the new start. */
char *trim( char *s );
const char *skipBlank( const char *s );
const char *trimBack( const char *beg, const char *end );
/* 0, CR, LF and EOF (1Ah) end a line. */
bool isEol( char c );

/* Decimal digits of n at d, terminated: the position of the 0. */
char *numStr( char *d, unsigned n );

/* a + sep + b into out (out may be a); no second separator when a ends with one. */
void joinPath( char *out, const char *a, const char *b, char sep );
void joinLocal( char *out, const char *a, const char *b );
void joinRemote( char *out, const char *a, const char *b );
/* Case-insensitive, '/' and '\' alike. */
bool samePath( const char *a, const char *b );

/* An 8.3 name without the characters DOS forbids. */
bool isDosName( const char *name );
/* .COM, .EXE or .BAT */
bool isExecutable( const char *name );

#endif
