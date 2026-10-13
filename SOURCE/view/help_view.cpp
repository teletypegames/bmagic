#include <stddef.h>

#include "colors.h"
#include "frame.h"
#include "help_view.h"
#include "../lib/ui/canvas.h"

enum { KeyCapWidth = 16 };
static const unsigned char AttrCap = 0x30;

/* A section title (key NULL), an empty row (both NULL), or a key and what it does. */
static const struct {
  const char *key, *desc;
} Lines[] = { { NULL, "Boot menu" },
              { "Enter", "the lines of the item (CONFIG.SYS, AUTOEXEC.BAT)" },
              { "Ins", "new item (empty, or a copy of another)" },
              { "Del", "delete the item, its block and section" },
              { "F3", "the item's name and menu text" },
              { "Ctrl+PgUp/PgDn", "move the item up / down in the menu" },
              { "F5", "make the item the default" },
              { "F6", "the menu: default, timeout, color, NumLock" },
              { "F7", "the Windows startup options (MSDOS.SYS)" },
              { "F8", "edit a whole file (Tab: the next file)" },
              { "F2", "review the changes and save them" },
              { "F4", "show the changes (not saved yet)" },
              { "F9", "the backups" },
              { "Esc", "quit (asks when something is not saved)" },
              { NULL, NULL },
              { NULL, "Lines" },
              { "Enter", "edit the line" },
              { "Ins", "a new line after the selected one" },
              { "Del", "delete the line" },
              { "Ctrl+PgUp/PgDn", "move the line up / down" },
              { "Esc", "back to the menu" },
              { NULL, NULL },
              { NULL, "Changes" },
              { "\x18\x19 PgUp PgDn", "scroll" },
              { "Tab", "the next file" },
              { "\x1B \x1A", "shift the lines left / right" },
              { "F2", "save (in the review before saving)" },
              { NULL, NULL },
              { NULL, "Backups" },
              { "Enter", "compare the backup with the files now" },
              { "Ins", "back up the files now" },
              { "R", "restore the backup (backs up first)" },
              { "Del", "delete the backup" },
              { NULL, NULL },
              { NULL, "Editing a line or a form" },
              { "\x1B \x1A Home End", "move in the text" },
              { "Bksp Del", "delete a character" },
              { "Ctrl+Y", "clear" },
              { "Enter / Esc", "OK / cancel" } };

static const char Keys[] = "~\x18\x19 PgUp PgDn~ Scroll  ~Esc~ Close";

HelpView::HelpView( Frame &frame ) : frame_( frame ) {}

int HelpView::lineCount() {
  return sizeof( Lines ) / sizeof( Lines[0] );
}

void HelpView::draw( int top ) {
  Canvas &canvas = frame_.canvas();
  frame_.title( "Boot Magic - keys", NULL );
  canvas.fillTo( Canvas::at( 1, 0 ), Canvas::at( 23, 0 ), AttrText );
  canvas.box( 1, 4, 23, 75, AttrFrame );
  canvas.boxTitle( 1, 6, "Keys", 20, AttrTitle );
  for ( int i = top; i < lineCount() && i - top < Rows; i++ ) {
    int row = 2 + i - top;
    int pos;
    if ( Lines[i].key ) {
      pos = canvas.put( Canvas::at( row, 7 ), ' ', AttrCap );
      pos = canvas.text( pos, Lines[i].key, KeyCapWidth - 2, AttrCap );
      pos = canvas.fillTo( pos, Canvas::at( row, 7 + KeyCapWidth ), AttrCap );
      canvas.text( pos + 2, Lines[i].desc, 48, AttrText );
    }
    else if ( Lines[i].desc ) {
      pos = canvas.text( Canvas::at( row, 7 ), Lines[i].desc, 30, AttrTitle );
      pos = canvas.put( pos, ' ', AttrTitle );
      do pos = canvas.put( pos, (char)0xC4, AttrFrame );
      while ( pos < Canvas::at( row, 73 ) );
    }
  }
  if ( top ) canvas.put( Canvas::at( 1, 71 ), 0x18, AttrFrame );
  if ( lineCount() - Rows > top ) canvas.put( Canvas::at( 23, 71 ), 0x19, AttrFrame );
  frame_.keys( Keys );
}
