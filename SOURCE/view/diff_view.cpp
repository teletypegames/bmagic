#include <stddef.h>
#include <string.h>

#include "colors.h"
#include "diff_view.h"
#include "frame.h"
#include "../lib/core/str.h"
#include "../lib/ui/canvas.h"
#include "../model/text_diff.h"

enum { NewCol = 6, MarkCol = 11, TextCol = 13 };

static const char ReviewKeys[] =
    "~F2~ Save  ~\x18\x19 PgUp PgDn~ Scroll  ~Tab~ Next file  ~\x1B\x1A~ Shift  ~Esc~ Back";
static const char ShowKeys[] = "~\x18\x19 PgUp PgDn~ Scroll  ~Tab~ Next file  ~\x1B\x1A~ Shift  ~Esc~ Back";

DiffView::DiffView( Frame &frame ) : frame_( frame ) {}

/* A line number (from 0) right-aligned in 4 columns at pos. */
static int lineNumber( Canvas &c, int pos, int line, unsigned char attr ) {
  char digits[8];
  if ( line < 0 ) return c.fillTo( pos, pos + 4, attr );
  numStr( digits, line + 1 );
  pos = c.fillTo( pos, pos + 4 - (int)strlen( digits ), attr );
  return c.text( pos, digits, 4, attr );
}

void DiffView::drawRow( int screenRow, const TextDiff &diff, int i, const TextFile &before, const TextFile &after,
                        int shift ) {
  Canvas &c = frame_.canvas();
  int pos = Canvas::at( screenRow, 0 ), end = Canvas::at( screenRow + 1, 0 );
  if ( i >= diff.count() ) {
    c.fillTo( pos, end, AttrText );
    return;
  }
  const DiffRow &row = diff.row( i );
  if ( row.kind == DiffSkip ) {
    pos = c.fillTo( pos, pos + MarkCol, AttrFrame );
    char text[40] = "\xFA\xFA\xFA ";
    char *d = numStr( text + 4, row.b );
    strcpy( d, row.b == 1 ? " line the same \xFA\xFA\xFA" : " lines the same \xFA\xFA\xFA" );
    pos = c.text( pos, text, 40, AttrFrame );
    c.fillTo( pos, end, AttrFrame );
    return;
  }
  unsigned char attr = row.kind == DiffRemoved ? AttrWarn : row.kind == DiffAdded ? AttrGood : AttrDim;
  pos = lineNumber( c, pos + 1, row.a, AttrDim );
  pos = lineNumber( c, pos + 1, row.b, AttrDim );
  pos = c.put( pos + 1, row.kind, attr );
  const char *text = row.kind == DiffAdded ? after.line( row.b ) : before.line( row.a );
  int len = strlen( text );
  pos = c.put( pos, ' ', attr );
  pos = c.text( pos, shift < len ? text + shift : "", TextWidth, row.kind == DiffSame ? AttrText : attr );
  c.fillTo( pos, end, AttrText );
}

void DiffView::draw( const char *title, const char *part, const TextDiff &diff, const TextFile &before,
                     const TextFile &after, int top, int shift, bool confirm ) {
  Canvas &c = frame_.canvas();
  frame_.title( title, part );
  c.fillTo( Canvas::at( 1, 0 ), Canvas::at( 23, 0 ), AttrText );
  int pos = c.textAt( 1, 1, " old  new", 9, AttrDim );
  pos = c.text( pos + 4, "removed: ", 9, AttrDim );
  pos = c.number( pos, diff.removed(), AttrWarn );
  pos = c.text( pos, "   added: ", 10, AttrDim );
  pos = c.number( pos, diff.added(), AttrGood );
  c.fillTo( pos, Canvas::at( 2, 0 ), AttrText );
  for ( int r = 0; r < Rows; r++ ) drawRow( FirstRow + r, diff, top + r, before, after, shift );
  if ( top + Rows < diff.count() ) c.textAt( 22, 1, "\x19 more", 10, AttrDim );
  frame_.keys( confirm ? ReviewKeys : ShowKeys );
}
