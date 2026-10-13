/*
   TCP connections. Only plain C types here: the DOS side also includes mTCP's types.h,
   whose uint16_t is not the one of <stdint.h>.
*/

#ifndef DOSLIB_PLATFORM_NETWORK_H
#define DOSLIB_PLATFORM_NETWORK_H

class Network {
public:
  enum { Closed = -2 };

  /* NULL, or the reason the stack cannot start. */
  virtual const char *start() = 0;
  virtual void stop() = 0;
  virtual bool resolve( const char *host, unsigned char ip[4] ) = 0;
  /* A handle >= 0, or -1. */
  virtual int connect( const unsigned char ip[4], unsigned port, unsigned recvSize ) = 0;
  /* The bytes sent (maybe 0), or -1. */
  virtual int send( int handle, const char *buf, unsigned len ) = 0;
  /* The bytes read, 0 when nothing has arrived yet, -1 on an error, Closed when the other
     side closed the connection and everything has been read. */
  virtual int receive( int handle, char *buf, unsigned len ) = 0;
  virtual void close( int handle ) = 0;
  /* Keeps the stack running while the program waits. */
  virtual void idle() = 0;
  /* Ctrl+Break / Ctrl+C was pressed. */
  virtual bool interrupted() = 0;
  virtual void clearInterrupted() = 0;
};

#endif
