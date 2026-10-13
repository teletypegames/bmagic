#include "boot_file.h"
#include "boot_menu.h"
#include "line_rows.h"

LineRows::LineRows() : count_( 0 ) {}

void LineRows::add( char kind, int file, int line ) {
  if ( count_ == Capacity ) return;
  LineRow &r = rows_[count_++];
  r.kind = kind;
  r.file = (char)file;
  r.line = line;
}

void LineRows::addPart( int file, int header, int end, const TextFile &text ) {
  add( RowHeader, file, header );
  if ( header < 0 && end < 0 ) {
    add( RowMissing, file, -1 );
    return;
  }
  for ( int i = header + 1; i < end && i < text.count(); i++ ) add( RowLine, file, i );
}

void LineRows::forItem( const BootMenu &menu, int item, const TextFile &config, const TextFile &autoexec ) {
  const MenuItem &it = menu.items[item];
  count_ = 0;
  addPart( ConfigSys, it.blockLine, it.blockLine >= 0 ? it.blockEnd : -1, config );
  addPart( AutoexecBat, it.labelLine, it.labelLine >= 0 ? it.labelEnd : -1, autoexec );
}

void LineRows::forFile( int file, const TextFile &text ) {
  count_ = 0;
  addPart( file, -1, text.count(), text );
}

int LineRows::find( int file, int line ) const {
  for ( int i = 0; i < count_; i++ ) {
    if ( rows_[i].kind == RowLine && rows_[i].file == file && rows_[i].line == line ) return i;
  }
  return -1;
}
