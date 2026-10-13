#ifndef DOSLIB_NET_FILE_SINK_H
#define DOSLIB_NET_FILE_SINK_H

#include <stdint.h>

#include "byte_sink.h"
#include "../core/crc32.h"

class FileSystem;

/* Into a file, counting the bytes and their CRC-32. */
class FileSink : public ByteSink {
public:
  explicit FileSink( FileSystem &fs );
  /* false when the file cannot be created */
  bool open( const char *path );
  /* false when the file could not be completed */
  bool close();
  bool write( const char *buf, unsigned len, Error &error );
  uint32_t crc() const;
  uint32_t size() const;

private:
  FileSystem &fs_;
  int handle_;
  Crc32 crc_;
  uint32_t size_;
};

#endif
