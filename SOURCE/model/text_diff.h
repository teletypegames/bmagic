/*
   The difference of two versions of a text file, line by line, as the rows of a unified
   diff: the removed and the added lines with 3 unchanged lines around them; the longer
   runs of unchanged lines become one skip row. Where the lines differ, the next place
   where they agree again is the nearest one (at most 50 lines ahead) where two lines in
   a row agree (or one, when there is no such place): small and fast, and for the edits
   of a boot file the same as the shortest diff.
*/

#ifndef BMAGIC_MODEL_TEXT_DIFF_H
#define BMAGIC_MODEL_TEXT_DIFF_H

#include "text_file.h"

enum DiffKind { DiffSame = ' ', DiffRemoved = '-', DiffAdded = '+', DiffSkip = '.' };

struct DiffRow {
  char kind;
  int a; /* the line in the old file (-1: an added line); a skip row: its first line */
  int b; /* the line in the new file (-1: a removed line); a skip row: how many lines */
};

class TextDiff {
public:
  enum { MaxRows = 2 * TextFile::MaxLines, Context = 3, Window = 50 };

  TextDiff();
  void compare( const TextFile &before, const TextFile &after );
  int count() const { return count_; }
  const DiffRow &row( int i ) const { return rows_[i]; }
  int added() const { return added_; }
  int removed() const { return removed_; }
  bool isSame() const { return added_ + removed_ == 0; }

private:
  void hash( const TextFile &file, unsigned short *out );
  bool same( int i, int j ) const;
  bool strong( int i, int j ) const;
  bool findSync( int i, int j, bool needStrong, int &x, int &y ) const;
  void emit( char kind, int a, int b );
  void fold();

  const TextFile *a_, *b_;
  unsigned short hashA_[TextFile::MaxLines], hashB_[TextFile::MaxLines];
  DiffRow rows_[MaxRows];
  char near_[MaxRows];
  int count_, added_, removed_;
};

#endif
