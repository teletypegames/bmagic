/*
   An FTP client for one control connection: passive mode, binary transfers. The data
   port is taken from the PASV reply, the address is the server's own: the reply's
   address is often an internal one behind NAT. Failures go into the Error.
*/

#ifndef DOSLIB_NET_FTP_CLIENT_H
#define DOSLIB_NET_FTP_CLIENT_H

class ByteSink;
class Clock;
class Error;
class Network;

/* Hears about a transfer in progress, at most twice a second. */
class TransferObserver {
public:
  virtual void transferred( const char *label, unsigned long bytes ) = 0;
};

class FtpClient {
public:
  FtpClient( Network &net, Clock &clock, Error &error );

  /* Connects and logs in (host names the server in the messages). */
  bool open( const char *host, const unsigned char ip[4], unsigned port, const char *user, const char *password );
  /* QUIT, and the connections closed. */
  void close();
  /* The reply code of the command, -1 when the connection failed. */
  int command( const char *fmt, ... );
  /* The last reply line. */
  const char *reply() const;
  /* The names in a folder (NLST) into buf, each terminated with a 0: the count, or -1. */
  int list( const char *path, char *buf, unsigned size );
  /* RETR into the sink: 0 when done, the reply code when the server refused (550), -1
     on an error. With an observer, the label names the transfer. */
  int retrieve( const char *path, ByteSink &sink, const char *label, TransferObserver *observer );

private:
  enum { ReplySize = 256, RecvSize = 512 };

  int connectTo( const unsigned char ip[4], unsigned port, unsigned recvSize );
  bool readLine( char *out, int size );
  int readReply();
  bool send( const char *fmt, ... );
  bool login( const char *user, const char *password );
  bool setBinaryMode();
  bool openData();
  bool parsePasvPort( unsigned &port );
  bool receiveData( ByteSink &sink, const char *label, TransferObserver *observer );
  int transfer( const char *command, const char *arg, ByteSink &sink, const char *label, TransferObserver *observer );
  void closeHandle( int &handle );

  Network &net_;
  Clock &clock_;
  Error &error_;
  unsigned char ip_[4];
  int ctrl_;
  int data_;
  char recv_[RecvSize];
  int recvLen_, recvPos_;
  char reply_[ReplySize];
  char xfer_[2048];
};

#endif
