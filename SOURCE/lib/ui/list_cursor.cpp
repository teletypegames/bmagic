#include "list_cursor.h"

ListCursor::ListCursor() : count_( 0 ), sel_( 0 ), top_( 0 ), rows_( 1 ) {}

void ListCursor::setRows( int rows ) {
  rows_ = rows;
}

void ListCursor::setCount( int count ) {
  count_ = count;
}

void ListCursor::place( int sel, int top ) {
  sel_ = sel;
  top_ = top;
}

void ListCursor::select( int index ) {
  sel_ = index;
  scrollToSelection();
}

void ListCursor::clamp() {
  if ( sel_ >= count_ ) sel_ = count_ - 1;
  if ( sel_ < 0 ) sel_ = 0;
  scrollToSelection();
}

void ListCursor::scrollToSelection() {
  if ( sel_ < top_ ) top_ = sel_;
  if ( sel_ >= top_ + rows_ ) top_ = sel_ - rows_ + 1;
}
