/*
   Editing one line of text in place: the arrows, Home / End, Backspace / Del and typing
   change it, Ctrl+Y clears it; the line scrolls to keep the cursor in a field of width
   columns.
*/

#ifndef BMAGIC_CONTROLLER_LINE_INPUT_H
#define BMAGIC_CONTROLLER_LINE_INPUT_H

class Speaker;

class LineInput {
public:
  explicit LineInput( Speaker &speaker );
  /* text: at least max + 1 bytes; the cursor goes to its end. */
  void start( char *text, int max, int width );
  /* false when the key does not edit (Enter, Esc, Tab, the other keys). */
  bool handle( unsigned key );
  int pos() const { return pos_; }
  int scroll() const { return scroll_; }

private:
  void insert( char c );
  void erase( int at );
  void follow();

  Speaker &speaker_;
  char *text_;
  int max_, width_, pos_, scroll_;
};

#endif
