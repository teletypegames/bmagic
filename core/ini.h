/*
   INI files: "KEY = value" lines. Lines starting with ';', '#' or '[' are not key lines;
   a key line has a '='. Keys compare case-insensitively; a value is trimmed and loses a
   pair of surrounding quotes. The text ends at a 0 or an EOF (1Ah) byte; a line ends at
   CR, LF or CR LF.
*/

#ifndef DOSLIB_CORE_INI_H
#define DOSLIB_CORE_INI_H

struct IniLine {
  const char *begin; /* the line, without its CR / LF */
  const char *end;
  const char *keyBegin; /* NULL: not a key line */
  const char *keyEnd;
  const char *value;
  int valueLength;

  bool isKeyLine() const;
  /* name is upper case */
  bool keyIs( const char *name ) const;
  void copyValue( char *dst, int size ) const;
};

class IniScanner {
public:
  /* A UTF-8 byte order mark at the start is skipped. */
  explicit IniScanner( const char *text );
  /* false at the end of the text */
  bool next( IniLine &line );
  /* Where the lines start (after the byte order mark). */
  const char *body() const;

private:
  const char *body_;
  const char *pos_;
};

#endif
