#include "crc32.h"

static uint32_t Table[256];
static bool TableReady = false;

static void makeTable() {
  for ( uint16_t i = 0; i < 256; i++ ) {
    uint32_t c = i;
    for ( int k = 0; k < 8; k++ ) c = ( c & 1 ) ? 0xEDB88320ul ^ ( c >> 1 ) : c >> 1;
    Table[i] = c;
  }
  TableReady = true;
}

Crc32::Crc32() : crc_( 0 ) {
  if ( !TableReady ) makeTable();
}

void Crc32::update( const void *buf, unsigned len ) {
  const uint8_t *p = (const uint8_t *)buf;
  uint32_t c = ~crc_;
  while ( len-- ) c = Table[( c ^ *p++ ) & 0xFF] ^ ( c >> 8 );
  crc_ = ~c;
}

uint32_t Crc32::value() const {
  return crc_;
}
