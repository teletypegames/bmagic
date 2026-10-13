/*
   Text encodings: the screen shows code page 437, the files may be in UTF-8.
*/

#ifndef DOSLIB_CORE_TEXT_H
#define DOSLIB_CORE_TEXT_H

/* UTF-8 -> code page 437 in place (s is terminated): the new length. Bytes that do not
   form a valid UTF-8 sequence are kept (text already in CP437); characters missing from
   CP437 become their base letter or '?'; a byte order mark is dropped. */
unsigned utf8To437( char *s );
/* Code page 437 -> UTF-8 at d (not terminated): the position after it. */
char *cp437ToUtf8( char *d, const char *s );
/* After a UTF-8 byte order mark. */
const char *skipBom( const char *s );

#endif
