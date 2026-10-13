#include <stddef.h>
#include <string.h>

#include "colors.h"
#include "frame.h"
#include "lines_view.h"
#include "../lib/ui/canvas.h"
#include "../lib/ui/list_cursor.h"
#include "../model/boot_docs.h"
#include "../model/line_rows.h"
#include "../model/text_file.h"

enum { TextCol = 7, TextWidth = 72 };

static const char ItemKeys[] =
    "~Enter~ Edit ~Ins~ Add ~Del~ Delete ~Ctrl+PgUp/Dn~ Move ~F4~ Changes ~F1~ Help ~Esc~ Back";
static const char FileKeys[] =
    "~Enter~ Edit ~Ins~ Add ~Del~ Delete ~Ctrl+PgUp/Dn~ Move ~Tab~ File ~F4~ Changes ~Esc~ Back";

LinesView::LinesView( Frame &frame ) : frame_( frame ) {}

void LinesView::drawRow( int screenRow, const LineRows &rows, int i, bool selected, const char *name,
                         const BootDocs &docs, char drive ) {
  Canvas &c = frame_.canvas();
  int pos = Canvas::at( screenRow, 1 ), end = Canvas::at( screenRow, 79 );
  if ( i >= rows.count() ) {
    c.fillTo( pos, end, AttrText );
    return;
  }
  const LineRow &row = rows.at( i );
  unsigned char a = selected ? AttrSelected : AttrText;
  pos = c.put( pos, ' ', a );
  if ( row.kind == RowHeader ) {
    unsigned char t = selected ? a : AttrTitle;
    if ( name && row.file == ConfigSys ) {
      pos = c.put( pos, '[', t );
      pos = c.text( pos, name, 60, t );
      pos = c.put( pos, ']', t );
    }
    else if ( name ) {
      pos = c.put( pos, ':', t );
      pos = c.text( pos, name, 60, t );
    }
    pos = c.text( pos, name ? "  in " : "", 5, selected ? a : AttrDim );
    pos = c.put( pos, drive, selected ? a : AttrDim );
    pos = c.text( pos, ":\\", 2, selected ? a : AttrDim );
    pos = c.text( pos, bootFileName( row.file ), 12, selected ? a : AttrDim );
  }
  else if ( row.kind == RowMissing ) {
    pos = c.text( pos, row.file == ConfigSys ? "(no block - Ins makes it)" : "(no section - Ins makes it)", 40,
                  selected ? a : AttrWarn );
  }
  else {
    const TextFile &text = *docs.edit[(int)row.file];
    char number[8];
    int n = row.line + 1, d = 0;
    char digits[8];
    do digits[d++] = (char)( '0' + n % 10 );
    while ( ( n /= 10 ) != 0 && d < 5 );
    int k = 0;
    while ( k < 4 - d ) number[k++] = ' ';
    while ( d ) number[k++] = digits[--d];
    number[k] = 0;
    pos = c.text( pos, number, 5, selected ? a : AttrDim );
    pos = c.fillTo( pos, Canvas::at( screenRow, TextCol ), a );
    pos = c.text( pos, text.line( row.line ), TextWidth, a );
  }
  c.fillTo( pos, end, a );
}

void LinesView::draw( const char *title, const char *name, const LineRows &rows, const ListCursor &cursor,
                      const BootDocs &docs, char drive, bool changed ) {
  Canvas &c = frame_.canvas();
  frame_.title( title, changed ? "* changed" : NULL );
  c.box( 1, 0, 22, 79, AttrFrame );
  c.boxTitle( 1, 2, name ? "Menu item" : "File", 20, AttrTitle );
  for ( int r = 0; r < Rows; r++ ) {
    int i = cursor.top() + r;
    drawRow( FirstRow + r, rows, i, i == cursor.sel(), name, docs, drive );
  }
  if ( cursor.top() > 0 ) c.put( Canvas::at( 1, 75 ), 0x18, AttrFrame );
  if ( cursor.top() + Rows < rows.count() ) c.put( Canvas::at( 22, 75 ), 0x19, AttrFrame );
  frame_.keys( name ? ItemKeys : FileKeys );
}
