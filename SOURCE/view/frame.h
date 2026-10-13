/*
   What every page has: the title bar (row 0), the status line (row 23: a message, a
   question, or a line being edited) and the keys (row 24). '~' in the keys toggles the
   highlight.
*/

#ifndef BMAGIC_VIEW_FRAME_H
#define BMAGIC_VIEW_FRAME_H

class Canvas;

enum { StatusRow = 23, KeysRow = 24 };

class Frame {
public:
  explicit Frame( Canvas &canvas );
  Canvas &canvas();
  void title( const char *left, const char *right );
  void keys( const char *keys );
  /* The status line: a message (NULL: empty), an error or a question in red. */
  void message( const char *text );
  void error( const char *text );
  /* An edit field on the status line: the label, the text from scroll on, the cursor. */
  void editLine( const char *label, const char *text, int scroll, int pos );
  /* The width of the edit field after a label. */
  static int editWidth( const char *label );
  void flush();

private:
  void status( const char *text, unsigned char attr );

  Canvas &canvas_;
};

#endif
