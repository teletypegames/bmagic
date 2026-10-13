/*
   A line editor page: the rows of LineRows (a header for each file part, its lines with
   their numbers, or a "missing" row), the selected one highlighted.
*/

#ifndef BMAGIC_VIEW_LINES_VIEW_H
#define BMAGIC_VIEW_LINES_VIEW_H

class Frame;
class LineRows;
class ListCursor;
struct BootDocs;

class LinesView {
public:
  enum { FirstRow = 2, Rows = 20 };

  explicit LinesView( Frame &frame );
  /* title: the page's title; name: the item's block name (NULL: whole files) */
  void draw( const char *title, const char *name, const LineRows &rows, const ListCursor &cursor, const BootDocs &docs,
             char drive, bool changed );

private:
  void drawRow( int screenRow, const LineRows &rows, int i, bool selected, const char *name, const BootDocs &docs,
                char drive );

  Frame &frame_;
};

#endif
