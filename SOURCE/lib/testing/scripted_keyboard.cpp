#include <stdlib.h>

#include "scripted_keyboard.h"

ScriptedKeyboard::ScriptedKeyboard( const char *keys, ScriptEnd &end ) : next_( keys ? keys : "" ), end_( end ) {}

void ScriptedKeyboard::begin() {}

void ScriptedKeyboard::end() {}

bool ScriptedKeyboard::ready() {
  return true;
}

unsigned ScriptedKeyboard::read() {
  while ( *next_ == ' ' || *next_ == '\r' || *next_ == '\n' ) next_++;
  if ( *next_ == 0 ) return end_.scriptEnded();
  char *after;
  unsigned key = (unsigned)strtoul( next_, &after, 16 );
  next_ = after;
  return key ? key : end_.scriptEnded();
}

bool ScriptedKeyboard::shiftDown() {
  return false;
}
