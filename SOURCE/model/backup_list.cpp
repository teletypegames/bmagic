#include <string.h>

#include "backup_list.h"
#include "../lib/core/ini.h"
#include "../lib/core/str.h"

static char *twoDigits( char *d, unsigned n ) {
  *d++ = (char)( '0' + n / 10 % 10 );
  *d++ = (char)( '0' + n % 10 );
  return d;
}

void BackupSet::setWhen( unsigned dosDate, unsigned dosTime ) {
  char *d = numStr( date, 1980 + ( dosDate >> 9 ) );
  *d++ = '-';
  d = twoDigits( d, ( dosDate >> 5 ) & 15 );
  *d++ = '-';
  d = twoDigits( d, dosDate & 31 );
  *d = 0;
  d = twoDigits( time, dosTime >> 11 );
  *d++ = ':';
  d = twoDigits( d, ( dosTime >> 5 ) & 63 );
  *d = 0;
}

static char *putKey( char *d, const char *key, const char *value ) {
  strcpy( d, key );
  strcat( d, "=" );
  strcat( d, value );
  strcat( d, "\r\n" );
  return d + strlen( d );
}

unsigned BackupSet::writeInfo( char *out ) const {
  char drv[2] = { drive, 0 };
  char *d = out;
  strcpy( d, "[backup]\r\n" );
  d += strlen( d );
  d = putKey( d, "date", date );
  d = putKey( d, "time", time );
  d = putKey( d, "drive", drv );
  d = putKey( d, "note", note );
  return d - out;
}

void BackupSet::readInfo( const char *text ) {
  IniScanner scanner( text );
  IniLine line;
  while ( scanner.next( line ) ) {
    if ( line.keyIs( "DATE" ) ) line.copyValue( date, sizeof( date ) );
    else if ( line.keyIs( "TIME" ) ) line.copyValue( time, sizeof( time ) );
    else if ( line.keyIs( "NOTE" ) ) line.copyValue( note, sizeof( note ) );
    else if ( line.keyIs( "DRIVE" ) && line.valueLength ) drive = upcase( line.value[0] );
  }
}

BackupList::BackupList() {
  clear();
}

void BackupList::clear() {
  count_ = 0;
  highest_ = 0;
}

bool BackupList::add( const BackupSet &set ) {
  if ( set.number > highest_ ) highest_ = set.number;
  int at = 0;
  while ( at < count_ && sets_[at].number > set.number ) at++;
  if ( at == Capacity ) return false;
  if ( count_ == Capacity ) count_--;
  memmove( sets_ + at + 1, sets_ + at, ( count_ - at ) * sizeof( sets_[0] ) );
  sets_[at] = set;
  count_++;
  return true;
}

int BackupList::nextNumber() const {
  return highest_ < MaxNumber ? highest_ + 1 : 0;
}
