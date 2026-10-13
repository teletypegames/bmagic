#include "console_screen.h"

ConsoleScreen::ConsoleScreen()
  : screen_( INVALID_HANDLE_VALUE ), oldScreen_( INVALID_HANDLE_VALUE ), dirtyFrom_( Cells ), dirtyTo_( 0 ) {
  for ( int i = 0; i < Cells; i++ ) {
    cells_[i].Char.AsciiChar = ' ';
    cells_[i].Attributes = 0x07;
  }
}

void ConsoleScreen::begin() {
  oldScreen_ = GetStdHandle( STD_OUTPUT_HANDLE );
  screen_ = CreateConsoleScreenBuffer( GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL );
  if ( screen_ == INVALID_HANDLE_VALUE ) screen_ = oldScreen_;
  COORD size = { Cols, Rows };
  SMALL_RECT window = { 0, 0, Cols - 1, Rows - 1 };
  SetConsoleScreenBufferSize( screen_, size );
  SetConsoleWindowInfo( screen_, TRUE, &window );
  SetConsoleActiveScreenBuffer( screen_ );
}

void ConsoleScreen::end() {
  SetConsoleActiveScreenBuffer( oldScreen_ );
  if ( screen_ != oldScreen_ ) CloseHandle( screen_ );
}

void ConsoleScreen::put( int pos, char c, unsigned char attr ) {
  if ( pos < 0 || pos >= Cells ) return;
  cells_[pos].Char.AsciiChar = c;
  cells_[pos].Attributes = attr;
  if ( pos < dirtyFrom_ ) dirtyFrom_ = pos;
  if ( pos >= dirtyTo_ ) dirtyTo_ = pos + 1;
}

unsigned ConsoleScreen::cell( int pos ) const {
  return ( (unsigned)( cells_[pos].Attributes & 0xFF ) << 8 ) | (unsigned char)cells_[pos].Char.AsciiChar;
}

void ConsoleScreen::flush() {
  if ( dirtyFrom_ >= dirtyTo_ ) return;
  COORD size = { Cols, Rows };
  COORD from = { 0, (SHORT)( dirtyFrom_ / Cols ) };
  SMALL_RECT rect = { 0, (SHORT)( dirtyFrom_ / Cols ), Cols - 1, (SHORT)( ( dirtyTo_ - 1 ) / Cols ) };
  WriteConsoleOutputA( screen_, cells_, size, from, &rect );
  dirtyFrom_ = Cells;
  dirtyTo_ = 0;
}

void ConsoleScreen::hideCursor() {
  CONSOLE_CURSOR_INFO cursor = { 25, FALSE };
  SetConsoleCursorInfo( screen_, &cursor );
}

void ConsoleScreen::moveCursor( int row, int col ) {
  COORD at = { (SHORT)col, (SHORT)row };
  SetConsoleCursorPosition( screen_, at );
}
