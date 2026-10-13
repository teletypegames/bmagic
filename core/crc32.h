#ifndef DOSLIB_CORE_CRC32_H
#define DOSLIB_CORE_CRC32_H

#include <stdint.h>

/* The CRC-32 of zlib and gzip. */
class Crc32 {
public:
  Crc32();
  void update( const void *buf, unsigned len );
  uint32_t value() const;

private:
  uint32_t crc_;
};

#endif
