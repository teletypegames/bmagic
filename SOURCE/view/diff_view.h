/*
   A diff page: the rows of a TextDiff with the line numbers of the old and the new
   version, the removed lines in red with '-', the added ones in green with '+', and a
   row for each run of unchanged lines that is left out.
*/

#ifndef BMAGIC_VIEW_DIFF_VIEW_H
#define BMAGIC_VIEW_DIFF_VIEW_H

class Frame;
class TextDiff;
class TextFile;

class DiffView {
public:
  enum { FirstRow = 2, Rows = 20, TextWidth = 66 };

  explicit DiffView( Frame &frame );
  /* title: what is compared; part: "CONFIG.SYS (1/2)"; confirm: the keys of the review
     before saving; shift: the first column of the text shown */
  void draw( const char *title, const char *part, const TextDiff &diff, const TextFile &before, const TextFile &after,
             int top, int shift, bool confirm );

private:
  void drawRow( int screenRow, const TextDiff &diff, int i, const TextFile &before, const TextFile &after, int shift );

  Frame &frame_;
};

#endif
