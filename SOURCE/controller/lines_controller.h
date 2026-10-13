/*
   The line editor: an item's [NAME] block and :NAME section, or a whole file (Tab: the
   next file). Enter edits the line on the status line (Enter keeps it, Esc drops the
   change), Ins adds a line after the selected one (after a header: at the top of the
   part; on a "missing" row it makes the block or section), Del deletes it, Ctrl+PgUp /
   Ctrl+PgDn move it within its part, F4 shows the changes, Esc goes back.
*/

#ifndef BMAGIC_CONTROLLER_LINES_CONTROLLER_H
#define BMAGIC_CONTROLLER_LINES_CONTROLLER_H

#include "../lib/ui/list_cursor.h"
#include "../model/limits.h"

class BootMenu;
class BootService;
class DiffController;
class Frame;
class HelpController;
class Keyboard;
class LineInput;
class LineRows;
class LinesView;
class MenuEditor;
class Speaker;
struct BootDocs;

class LinesController {
public:
  LinesController( LinesView &view, Frame &frame, Keyboard &keyboard, Speaker &speaker, LineInput &input,
                   BootDocs &docs, BootMenu &menu, MenuEditor &editor, LineRows &rows, BootService &boot,
                   DiffController &diffs, HelpController &help, char drive );
  void runItem( int item );
  void runFile( int file );

private:
  void run();
  bool rebuild();
  void draw();
  void selectLine( int file, int line );
  bool edit( int file, int line, bool added );
  void insert();
  void remove();
  void move( int delta );
  void nextFile();
  bool handleKey( unsigned key );

  LinesView &view_;
  Frame &frame_;
  Keyboard &keyboard_;
  Speaker &speaker_;
  LineInput &input_;
  BootDocs &docs_;
  BootMenu &menu_;
  MenuEditor &editor_;
  LineRows &rows_;
  BootService &boot_;
  DiffController &diffs_;
  HelpController &help_;
  char drive_;
  int file_;                /* a whole file, or -1 for an item */
  char name_[MaxBlock + 1]; /* the item's block name */
  char title_[MaxBlock + 40];
  ListCursor cursor_;
  const char *message_;
  bool error_;
};

#endif
