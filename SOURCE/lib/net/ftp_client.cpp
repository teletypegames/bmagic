#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "byte_sink.h"
#include "ftp_client.h"
#include "../core/error.h"
#include "../core/str.h"
#include "../platform/clock.h"
#include "../platform/network.h"

static const unsigned CtrlRecvSize = 1024;
static const unsigned DataRecvSize = 8192;
static const unsigned long ReplyTimeout = 20000ul;
static const unsigned long DataTimeout = 30000ul;
static const unsigned long ProgressEvery = 500ul;

FtpClient::FtpClient( Network &net, Clock &clock, Error &error )
  : net_( net ), clock_( clock ), error_( error ), ctrl_( -1 ), data_( -1 ), recvLen_( 0 ), recvPos_( 0 ) {
  reply_[0] = 0;
}

const char *FtpClient::reply() const {
  return reply_;
}

void FtpClient::closeHandle( int &handle ) {
  if ( handle >= 0 ) net_.close( handle );
  handle = -1;
}

/* Three tries: on DOS a local port from an earlier run may still be in use. */
int FtpClient::connectTo( const unsigned char ip[4], unsigned port, unsigned recvSize ) {
  for ( int tries = 0; tries < 3 && !net_.interrupted(); tries++ ) {
    int h = net_.connect( ip, port, recvSize );
    if ( h >= 0 ) return h;
  }
  return -1;
}

bool FtpClient::readLine( char *out, int size ) {
  int len = 0;
  unsigned long start = clock_.ms();
  for ( ;; ) {
    if ( recvPos_ < recvLen_ ) {
      char c = recv_[recvPos_++];
      if ( c == '\n' ) {
        if ( len > 0 && out[len - 1] == '\r' ) len--;
        out[len] = 0;
        return true;
      }
      if ( len < size - 1 ) out[len++] = c;
      continue;
    }
    if ( net_.interrupted() ) {
      error_.set( "Interrupted" );
      return false;
    }
    int n = net_.receive( ctrl_, recv_, sizeof( recv_ ) );
    if ( n == -1 ) {
      error_.set( "Connection error" );
      return false;
    }
    if ( n == Network::Closed ) {
      error_.set( "Server closed the connection" );
      return false;
    }
    if ( n > 0 ) {
      recvLen_ = n;
      recvPos_ = 0;
      continue;
    }
    if ( clock_.ms() - start > ReplyTimeout ) {
      error_.set( "Server does not answer" );
      return false;
    }
  }
}

int FtpClient::readReply() {
  if ( !readLine( reply_, sizeof( reply_ ) ) ) return -1;
  int code = atoi( reply_ );
  if ( strlen( reply_ ) >= 4 && reply_[3] == '-' ) {
    char first[4];
    memcpy( first, reply_, 3 );
    first[3] = 0;
    do {
      if ( !readLine( reply_, sizeof( reply_ ) ) ) return -1;
    } while ( !( strncmp( reply_, first, 3 ) == 0 && reply_[3] == ' ' ) );
  }
  return code;
}

bool FtpClient::send( const char *fmt, ... ) {
  char buf[200];
  va_list ap;
  va_start( ap, fmt );
  vsnprintf( buf, sizeof( buf ) - 2, fmt, ap );
  va_end( ap );
  strcat( buf, "\r\n" );

  unsigned len = strlen( buf ), sent = 0;
  unsigned long start = clock_.ms();
  while ( sent < len ) {
    int n = net_.send( ctrl_, buf + sent, len - sent );
    if ( n < 0 ) {
      error_.set( "Connection error" );
      return false;
    }
    sent += n;
    if ( sent < len ) {
      if ( clock_.ms() - start > ReplyTimeout ) {
        error_.set( "Cannot send to server" );
        return false;
      }
      net_.idle();
    }
  }
  return true;
}

int FtpClient::command( const char *fmt, ... ) {
  char buf[200];
  va_list ap;
  va_start( ap, fmt );
  vsnprintf( buf, sizeof( buf ), fmt, ap );
  va_end( ap );
  if ( !send( "%s", buf ) ) return -1;
  return readReply();
}

bool FtpClient::login( const char *user, const char *password ) {
  int code = command( "USER %s", user );
  if ( code == 331 ) code = command( "PASS %s", password );
  if ( code != 230 ) {
    error_.set( "Login failed: %s", reply_ );
    return false;
  }
  return true;
}

bool FtpClient::setBinaryMode() {
  if ( command( "TYPE I" ) != 200 ) {
    error_.set( "TYPE I failed: %s", reply_ );
    return false;
  }
  return true;
}

bool FtpClient::open( const char *host, const unsigned char ip[4], unsigned port, const char *user,
                      const char *password ) {
  memcpy( ip_, ip, 4 );
  recvLen_ = recvPos_ = 0;
  ctrl_ = connectTo( ip_, port, CtrlRecvSize );
  if ( ctrl_ < 0 ) {
    error_.set( "Cannot connect to %s:%u", host, port );
    return false;
  }
  bool ok = readReply() == 220 && login( user, password ) && setBinaryMode();
  if ( !ok ) {
    if ( !error_.isSet() ) error_.set( "FTP error: %s", reply_ );
    close();
  }
  return ok;
}

void FtpClient::close() {
  if ( ctrl_ >= 0 && send( "QUIT" ) ) readReply();
  closeHandle( data_ );
  closeHandle( ctrl_ );
}

bool FtpClient::parsePasvPort( unsigned &port ) {
  char *p = strchr( reply_, '(' );
  if ( p == NULL ) {
    p = reply_ + 3;
    while ( *p && ( *p < '0' || *p > '9' ) ) p++;
  }
  else p++;
  unsigned a, b, c, d, hi, lo;
  if ( sscanf( p, "%u,%u,%u,%u,%u,%u", &a, &b, &c, &d, &hi, &lo ) != 6 ) {
    error_.set( "Bad PASV reply: %s", reply_ );
    return false;
  }
  port = ( hi << 8 ) | lo;
  return true;
}

bool FtpClient::openData() {
  int code = command( "PASV" );
  if ( code != 227 ) {
    if ( code > 0 ) error_.set( "PASV failed: %s", reply_ );
    return false;
  }
  unsigned port;
  if ( !parsePasvPort( port ) ) return false;
  data_ = connectTo( ip_, port, DataRecvSize );
  if ( data_ < 0 ) {
    error_.set( "Data connection failed" );
    return false;
  }
  return true;
}

bool FtpClient::receiveData( ByteSink &sink, const char *label, TransferObserver *observer ) {
  unsigned long last = clock_.ms();
  unsigned long shown = last;
  unsigned long total = 0;
  for ( ;; ) {
    if ( net_.interrupted() ) {
      error_.set( "Interrupted" );
      return false;
    }
    int n = net_.receive( data_, xfer_, sizeof( xfer_ ) );
    if ( n == -1 ) {
      error_.set( "Data connection error" );
      return false;
    }
    if ( n == Network::Closed ) return true;
    if ( n > 0 ) {
      total += n;
      if ( !sink.write( xfer_, n, error_ ) ) return false;
      last = clock_.ms();
      if ( observer && last - shown > ProgressEvery ) {
        observer->transferred( label, total );
        shown = last;
      }
      continue;
    }
    if ( clock_.ms() - last > DataTimeout ) {
      error_.set( "Transfer timed out" );
      return false;
    }
  }
}

int FtpClient::transfer( const char *command, const char *arg, ByteSink &sink, const char *label,
                         TransferObserver *observer ) {
  if ( !openData() ) return -1;
  int code = this->command( command, arg );
  if ( code != 150 && code != 125 ) {
    closeHandle( data_ );
    return code < 0 ? -1 : code;
  }
  bool ok = receiveData( sink, label, observer );
  closeHandle( data_ );
  if ( !ok ) return -1;
  code = readReply();
  if ( code != 226 && code != 250 ) {
    if ( code > 0 ) error_.set( "Transfer failed: %s", reply_ );
    return -1;
  }
  return 0;
}

int FtpClient::retrieve( const char *path, ByteSink &sink, const char *label, TransferObserver *observer ) {
  return transfer( "RETR %s", path, sink, label, observer );
}

static const char *cleanName( char *line ) {
  char *name = trim( line );
  char *slash = strrchr( name, '/' );
  if ( slash ) name = slash + 1;
  if ( !*name || strcmp( name, "." ) == 0 || strcmp( name, ".." ) == 0 ) return NULL;
  return name;
}

static int compactNames( char *buf ) {
  int count = 0;
  char *out = buf;
  char *line = buf;
  while ( *line ) {
    char *next = strchr( line, '\n' );
    if ( next ) *next++ = 0;
    else next = line + strlen( line );
    const char *name = cleanName( line );
    if ( name ) {
      unsigned len = strlen( name ) + 1;
      memmove( out, name, len );
      out += len;
      count++;
    }
    line = next;
  }
  *out = 0;
  return count;
}

int FtpClient::list( const char *path, char *buf, unsigned size ) {
  int code = command( "CWD %s", path );
  if ( code != 250 ) {
    if ( code > 0 ) error_.set( "No folder %s on the server", path );
    return -1;
  }
  MemorySink sink( buf, size );
  code = transfer( "NLST", "", sink, NULL, NULL );
  if ( code == 450 || code == 550 ) return 0;
  if ( code ) {
    if ( code > 0 ) error_.set( "Listing failed: %s", reply_ );
    return -1;
  }
  return compactNames( buf );
}
