#include <stddef.h>
#include <string.h>

#include "line_input.h"
#include "../lib/platform/speaker.h"
#include "../lib/ui/keys.h"

LineInput::LineInput( Speaker &speaker )
  : speaker_( speaker ), text_( NULL ), max_( 0 ), width_( 1 ), pos_( 0 ), scroll_( 0 ) {}

void LineInput::start( char *text, int max, int width ) {
  text_ = text;
  max_ = max;
  width_ = width;
  pos_ = strlen( text );
  scroll_ = 0;
  follow();
}

void LineInput::follow() {
  if ( pos_ < scroll_ ) scroll_ = pos_;
  if ( pos_ - scroll_ >= width_ ) scroll_ = pos_ - width_ + 1;
}

void LineInput::insert( char c ) {
  int len = strlen( text_ );
  if ( len >= max_ ) {
    speaker_.beep();
    return;
  }
  memmove( text_ + pos_ + 1, text_ + pos_, len - pos_ + 1 );
  text_[pos_++] = c;
}

void LineInput::erase( int at ) {
  memmove( text_ + at, text_ + at + 1, strlen( text_ + at ) );
}

bool LineInput::handle( unsigned key ) {
  unsigned char ch = keyAscii( key ), scan = keyScan( key );
  if ( ch == KeyBackspace ) {
    if ( pos_ ) erase( --pos_ );
  }
  else if ( ch == KeyCtrlY ) {
    text_[0] = 0;
    pos_ = 0;
  }
  else if ( isExtendedKey( key ) ) {
    switch ( scan ) {
      case ScanLeft:
        if ( pos_ ) pos_--;
        break;
      case ScanRight:
        if ( text_[pos_] ) pos_++;
        break;
      case ScanHome: pos_ = 0; break;
      case ScanEnd: pos_ = strlen( text_ ); break;
      case ScanDel:
        if ( text_[pos_] ) erase( pos_ );
        break;
      default: return false;
    }
  }
  else if ( ch >= ' ' && ch != KeyDelChar ) insert( (char)ch );
  else return false;
  follow();
  return true;
}
