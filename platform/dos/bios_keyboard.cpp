#include <bios.h>

#include "bios_keyboard.h"

void BiosKeyboard::begin() {}

void BiosKeyboard::end() {}

bool BiosKeyboard::ready() {
  return _bios_keybrd( _KEYBRD_READY ) != 0;
}

unsigned BiosKeyboard::read() {
  return _bios_keybrd( _KEYBRD_READ );
}

bool BiosKeyboard::shiftDown() {
  return ( _bios_keybrd( _KEYBRD_SHIFTSTATUS ) & 3 ) != 0;
}
