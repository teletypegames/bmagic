#include "console_break.h"
#include "console_keyboard.h"

ConsoleKeyboard::ConsoleKeyboard() : input_( INVALID_HANDLE_VALUE ), oldMode_( 0 ), pending_( 0 ), shift_( false ) {}

void ConsoleKeyboard::begin() {
  input_ = GetStdHandle( STD_INPUT_HANDLE );
  GetConsoleMode( input_, &oldMode_ );
  SetConsoleMode( input_, 0 );
  consoleBreakCatch();
}

void ConsoleKeyboard::end() {
  SetConsoleMode( input_, oldMode_ );
}

static bool isShiftKey( unsigned scan ) {
  return scan == 0x2A || scan == 0x36 || scan == 0x1D || scan == 0x38 || scan == 0x3A;
}

bool ConsoleKeyboard::ready() {
  INPUT_RECORD rec;
  DWORD count;
  while ( pending_ == 0 ) {
    if ( !GetNumberOfConsoleInputEvents( input_, &count ) || count == 0 ) return false;
    if ( !ReadConsoleInputA( input_, &rec, 1, &count ) || count == 0 ) return false;
    if ( rec.EventType != KEY_EVENT || !rec.Event.KeyEvent.bKeyDown ) continue;
    unsigned scan = rec.Event.KeyEvent.wVirtualScanCode & 0xFF;
    unsigned char ch = (unsigned char)rec.Event.KeyEvent.uChar.AsciiChar;
    shift_ = ( rec.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED ) != 0;
    if ( ch == 0 && isShiftKey( scan ) ) continue;
    pending_ = ( scan << 8 ) | ch;
  }
  return true;
}

unsigned ConsoleKeyboard::read() {
  while ( !ready() ) Sleep( 10 );
  unsigned key = pending_;
  pending_ = 0;
  return key;
}

bool ConsoleKeyboard::shiftDown() {
  return shift_;
}
