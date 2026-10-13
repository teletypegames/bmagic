#include <stddef.h>

#include "colors.h"
#include "form_view.h"
#include "frame.h"
#include "../lib/ui/canvas.h"

enum { Left = 4, Right = 75, LabelCol = 7, ValueCol = 24 };

static const char Keys[] = "~Tab~/~\x18\x19~ Field  ~Enter~ Next, OK  ~F2~ OK  ~Esc~ Cancel  ~Ctrl+Y~ Clear";

FormView::FormView( Frame &frame ) : frame_( frame ) {}

int FormView::width( const FormField &field ) {
  return field.max < FieldWidth ? field.max + 1 : FieldWidth;
}

void FormView::draw( const char *title, const FormField *fields, int count, const FormCursor &cursor,
                     const char *error ) {
  Canvas &c = frame_.canvas();
  int top = 11 - count, bottom = top + 2 * count + 4;
  c.box( top, Left, bottom, Right, AttrFrame );
  c.boxTitle( top, Left + 2, title, 40, AttrTitle );
  for ( int f = 0; f < count; f++ ) {
    int row = top + 2 + 2 * f;
    c.fillTo( Canvas::at( row - 1, Left + 1 ), Canvas::at( row - 1, Right ), AttrText );
    int pos = c.fillTo( Canvas::at( row, Left + 1 ), Canvas::at( row, LabelCol ), AttrText );
    pos = c.text( pos, fields[f].label, ValueCol - LabelCol - 1, f == cursor.field ? AttrTitle : AttrDim );
    pos = c.fillTo( pos, Canvas::at( row, ValueCol ), AttrText );
    const char *v = fields[f].value;
    int w = width( fields[f] );
    unsigned char a = f == cursor.field ? 0x70 : 0x07;
    if ( f == cursor.field ) v += cursor.scroll;
    int n = 0;
    for ( ; n < w && v[n]; n++ ) pos = c.put( pos, v[n], a );
    for ( ; n < w; n++ ) pos = c.put( pos, ' ', a );
    c.fillTo( pos, Canvas::at( row, Right ), AttrText );
  }
  c.separator( bottom - 2, Left, Right, AttrFrame );
  int pos = c.fillTo( Canvas::at( bottom - 1, Left + 1 ), Canvas::at( bottom - 1, Left + 2 ), AttrText );
  pos = c.text( pos, error ? error : fields[cursor.field].hint, Right - Left - 3, error ? AttrWarn : AttrDim );
  c.fillTo( pos, Canvas::at( bottom - 1, Right ), AttrText );
  frame_.keys( Keys );
  frame_.message( NULL );
  int row = top + 2 + 2 * cursor.field;
  c.screen().moveCursor( row, ValueCol + cursor.pos - cursor.scroll );
}
