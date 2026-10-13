#include <stdio.h>
#include <string.h>

#include "../core/error.h"
#include "../net/byte_sink.h"
#include "../net/ftp_client.h"
#include "../platform/network.h"
#include "../testing/host/fake_clock.h"
#include "../testing/host/minitest.h"

/* An FTP server on two handles: 0 is the control connection, 1 the data connection.
   Its files are "path=content" entries. */
class FakeFtpServer : public Network {
public:
  FakeFtpServer() : replyLen_( 0 ), replyPos_( 0 ), dataLen_( 0 ), dataPos_( 0 ), fileCount_( 0 ) { commands[0] = 0; }
  void addFile( const char *path, const char *content ) {
    paths_[fileCount_] = path;
    contents_[fileCount_++] = content;
  }
  const char *start() { return NULL; }
  void stop() {}
  bool resolve( const char *, unsigned char ip[4] ) {
    memset( ip, 1, 4 );
    return true;
  }
  int connect( const unsigned char *, unsigned port, unsigned ) {
    if ( port == 21 ) {
      say( "220 hello" );
      return 0;
    }
    if ( port != 4 * 256 + 1 ) return -1;
    dataPos_ = 0;
    return 1;
  }
  int send( int, const char *buf, unsigned len ) {
    char line[200];
    memcpy( line, buf, len - 2 );
    line[len - 2] = 0;
    strcat( commands, line );
    strcat( commands, "|" );
    answer( line );
    return (int)len;
  }
  int receive( int h, char *buf, unsigned len ) {
    if ( h == 0 ) {
      unsigned n = replyLen_ - replyPos_ < len ? replyLen_ - replyPos_ : len;
      memcpy( buf, reply_ + replyPos_, n );
      replyPos_ += n;
      return (int)n;
    }
    if ( dataPos_ >= dataLen_ ) return Closed;
    unsigned n = dataLen_ - dataPos_ < len ? dataLen_ - dataPos_ : len;
    memcpy( buf, data_ + dataPos_, n );
    dataPos_ += n;
    return (int)n;
  }
  void close( int ) {}
  void idle() {}
  bool interrupted() { return false; }
  void clearInterrupted() {}

  char commands[400];

private:
  void say( const char *s ) { replyLen_ += snprintf( reply_ + replyLen_, sizeof( reply_ ) - replyLen_, "%s\r\n", s ); }
  void setData( const char *s ) {
    strcpy( data_, s );
    dataLen_ = strlen( s );
  }
  const char *file( const char *path ) const {
    for ( int i = 0; i < fileCount_; i++ ) {
      if ( strcmp( paths_[i], path ) == 0 ) return contents_[i];
    }
    return NULL;
  }
  void answer( const char *line ) {
    const char *space = strchr( line, ' ' );
    const char *arg = space ? space + 1 : "";
    if ( strncmp( line, "USER", 4 ) == 0 ) say( "331 password" );
    else if ( strncmp( line, "PASS", 4 ) == 0 ) say( "230-welcome\r\n230 logged in" );
    else if ( strcmp( line, "TYPE I" ) == 0 ) say( "200 binary" );
    else if ( strcmp( line, "PASV" ) == 0 ) say( "227 Entering Passive Mode (10,0,0,9,4,1)" );
    else if ( strncmp( line, "CWD", 3 ) == 0 ) say( strcmp( arg, "/" ) == 0 ? "250 ok" : "550 no" );
    else if ( strcmp( line, "NLST" ) == 0 ) {
      setData( "/HELLO\r\n./BIG\r\n.\r\n" );
      say( "150 here\r\n226 done" );
    }
    else if ( strncmp( line, "RETR", 4 ) == 0 ) {
      if ( file( arg ) == NULL ) say( "550 missing" );
      else {
        setData( file( arg ) );
        say( "150 here\r\n226 done" );
      }
    }
    else if ( strcmp( line, "QUIT" ) == 0 ) say( "221 bye" );
    else say( "500 what" );
  }

  char reply_[2000];
  unsigned replyLen_, replyPos_;
  char data_[200];
  unsigned dataLen_, dataPos_;
  const char *paths_[4];
  const char *contents_[4];
  int fileCount_;
};

static const unsigned char ServerIp[4] = { 1, 2, 3, 4 };

TEST( ftpLogsInAndLists ) {
  FakeFtpServer server;
  FakeClock clock;
  Error error;
  FtpClient ftp( server, clock, error );
  CHECK( ftp.open( "server", ServerIp, 21, "anonymous", "me@" ) );
  char buf[64];
  CHECK_INT( ftp.list( "/", buf, sizeof( buf ) ), 2 );
  CHECK_STR( buf, "HELLO" );
  CHECK_STR( buf + 6, "BIG" );
  ftp.close();
  CHECK_STR( server.commands, "USER anonymous|PASS me@|TYPE I|CWD /|PASV|NLST|QUIT|" );
}

TEST( ftpRetrievesOrTellsTheRefusal ) {
  FakeFtpServer server;
  server.addFile( "/HELLO/INFO.INI", "NAME=Hello\r\n" );
  FakeClock clock;
  Error error;
  FtpClient ftp( server, clock, error );
  CHECK( ftp.open( "server", ServerIp, 21, "anonymous", "me@" ) );
  char buf[64];
  MemorySink sink( buf, sizeof( buf ) );
  CHECK_INT( ftp.retrieve( "/HELLO/INFO.INI", sink, NULL, NULL ), 0 );
  CHECK_STR( buf, "NAME=Hello\r\n" );
  MemorySink missing( buf, sizeof( buf ) );
  CHECK_INT( ftp.retrieve( "/HELLO/NONE", missing, NULL, NULL ), 550 );
}

TEST( ftpTellsAFailedConnection ) {
  FakeFtpServer server;
  FakeClock clock;
  Error error;
  FtpClient ftp( server, clock, error );
  CHECK( !ftp.open( "server", ServerIp, 2121, "anonymous", "me@" ) );
  CHECK_STR( error.text(), "Cannot connect to server:2121" );
}

TEST( ftpListOfAMissingFolderFails ) {
  FakeFtpServer server;
  FakeClock clock;
  Error error;
  FtpClient ftp( server, clock, error );
  CHECK( ftp.open( "server", ServerIp, 21, "anonymous", "me@" ) );
  char buf[64];
  CHECK_INT( ftp.list( "/NOPE", buf, sizeof( buf ) ), -1 );
  CHECK_STR( error.text(), "No folder /NOPE on the server" );
}
