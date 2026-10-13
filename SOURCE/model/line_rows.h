/*
   The rows of a line editor page: a header for each file part, then its lines. An item
   shows its [NAME] block of CONFIG.SYS and its :NAME section of AUTOEXEC.BAT (or a
   "missing" row in place of one); a whole file shows all its lines.
*/

#ifndef BMAGIC_MODEL_LINE_ROWS_H
#define BMAGIC_MODEL_LINE_ROWS_H

#include "text_file.h"

class BootMenu;

enum RowKind { RowHeader, RowLine, RowMissing };

struct LineRow {
  char kind;
  char file; /* BootFile */
  int line;  /* RowLine: the line; RowHeader: the [NAME] / :NAME line, -1 for a whole file */
};

class LineRows {
public:
  enum { Capacity = 2 * TextFile::MaxLines + 4 };

  LineRows();
  void forItem( const BootMenu &menu, int item, const TextFile &config, const TextFile &autoexec );
  void forFile( int file, const TextFile &text );
  int count() const { return count_; }
  const LineRow &at( int i ) const { return rows_[i]; }
  /* The row of a line, or -1. */
  int find( int file, int line ) const;

private:
  void add( char kind, int file, int line );
  void addPart( int file, int header, int end, const TextFile &text );

  LineRow rows_[Capacity];
  int count_;
};

#endif
