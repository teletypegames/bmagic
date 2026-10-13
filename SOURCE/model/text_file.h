/*
   A text file as lines, for editing it line by line. Lines end at CR LF, LF or CR; an
   EOF byte (1Ah) ends the text, the bytes after it are dropped. write() gives the file
   back as it was read when nothing changed: the same line ends (CR LF, or LF when the
   file had only those), the line end after the last line only if it had one, the EOF
   byte if it had one. The lines are kept in a pool; a changed line takes new room in it,
   and the pool is compacted when it is full.
*/

#ifndef BMAGIC_MODEL_TEXT_FILE_H
#define BMAGIC_MODEL_TEXT_FILE_H

class TextFile {
public:
  enum { MaxLines = 500, PoolSize = 16384, OutSize = PoolSize + 2 * MaxLines + 1 };

  TextFile();
  /* An empty file (a new one: write() ends its lines with CR LF). */
  void clear();
  /* len bytes of text; false when it has more lines than MaxLines or does not fit. */
  bool load( const char *text, unsigned len );
  /* A 0 byte in the first len bytes: not a text file. */
  static bool isBinary( const char *text, unsigned len );

  int count() const { return count_; }
  const char *line( int i ) const { return pool_ + start_[i]; }
  /* false when the file is full (MaxLines or the pool). */
  bool insert( int at, const char *text );
  /* A copy of line i (its number before the insert) at at. */
  bool duplicate( int at, int i );
  bool replace( int i, const char *text );
  void remove( int i );
  void swap( int i, int j );
  bool sameAs( const TextFile &other ) const;
  /* The file into out (OutSize bytes): its length. */
  unsigned write( char *out ) const;

private:
  bool store( const char *text, unsigned short &at );
  void compact();

  unsigned short start_[MaxLines];
  unsigned short order_[MaxLines];
  int count_;
  unsigned used_;
  bool lfOnly_, lastEnds_, eof_;
  char pool_[PoolSize];
};

#endif
