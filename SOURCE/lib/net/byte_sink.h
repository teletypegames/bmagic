#ifndef DOSLIB_NET_BYTE_SINK_H
#define DOSLIB_NET_BYTE_SINK_H

class Error;

/* Where a download goes. */
class ByteSink {
public:
  virtual bool write( const char *buf, unsigned len, Error &error ) = 0;
};

/* Into a buffer, terminated with a 0; what does not fit is dropped. */
class MemorySink : public ByteSink {
public:
  MemorySink( char *buf, unsigned size );
  bool write( const char *buf, unsigned len, Error &error );

private:
  char *buf_;
  unsigned size_;
  unsigned len_;
};

#endif
