#include <stddef.h>

#include "backup_view.h"
#include "colors.h"
#include "frame.h"
#include "../lib/ui/canvas.h"
#include "../lib/ui/list_cursor.h"
#include "../model/backup_list.h"
#include "../model/boot_file.h"

enum { DateCol = 8, TimeCol = 20, NoteCol = 27, FilesCol = 56 };

static const char Keys[] = "~Enter~ Compare ~Ins~ Back up now ~R~ Restore ~Del~ Delete ~Esc~ Back";
static const char *const Short[BootFileCount] = { "CONFIG", "AUTOEXEC", "MSDOS" };

BackupView::BackupView( Frame &frame ) : frame_( frame ) {}

void BackupView::draw( BackupList &sets, const ListCursor &cursor, const char *folder ) {
  Canvas &c = frame_.canvas();
  frame_.title( "Boot Magic - backups", folder );
  c.box( 1, 0, 22, 79, AttrFrame );
  c.boxTitle( 1, 2, "Backups", 20, AttrTitle );
  c.textAt( 2, 2, "Set", 3, AttrTitle );
  c.textAt( 2, DateCol, "Date", 10, AttrTitle );
  c.textAt( 2, TimeCol, "Time", 5, AttrTitle );
  c.textAt( 2, NoteCol, "Note", 30, AttrTitle );
  c.textAt( 2, FilesCol, "Files", 20, AttrTitle );
  if ( sets.count() == 0 ) c.textAt( FirstRow + 1, 4, "No backups yet. Ins makes one now.", 60, AttrText );
  for ( int r = 0; r < Rows; r++ ) {
    int i = cursor.top() + r;
    if ( i >= sets.count() ) break;
    BackupSet &set = sets.at( i );
    unsigned char a = i == cursor.sel() ? AttrSelected : AttrText;
    int row = FirstRow + r;
    int pos = c.put( Canvas::at( row, 1 ), ' ', a );
    pos = c.put( pos, (char)( '0' + set.number / 100 % 10 ), a );
    pos = c.put( pos, (char)( '0' + set.number / 10 % 10 ), a );
    pos = c.put( pos, (char)( '0' + set.number % 10 ), a );
    pos = c.fillTo( pos, Canvas::at( row, DateCol ), a );
    pos = c.text( pos, set.date, 10, a );
    pos = c.fillTo( pos, Canvas::at( row, TimeCol ), a );
    pos = c.text( pos, set.time, 5, a );
    pos = c.fillTo( pos, Canvas::at( row, NoteCol ), a );
    pos = c.text( pos, set.note, FilesCol - NoteCol - 1, a );
    pos = c.fillTo( pos, Canvas::at( row, FilesCol ), a );
    for ( int f = 0; f < BootFileCount; f++ ) {
      if ( !set.has( f ) ) continue;
      pos = c.text( pos, Short[f], 8, a );
      pos = c.put( pos, ' ', a );
    }
    c.fillTo( pos, Canvas::at( row, 79 ), a );
  }
  if ( cursor.top() > 0 ) c.put( Canvas::at( 1, 75 ), 0x18, AttrFrame );
  if ( cursor.top() + Rows < sets.count() ) c.put( Canvas::at( 22, 75 ), 0x19, AttrFrame );
  frame_.keys( Keys );
}
