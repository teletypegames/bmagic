#include <stddef.h>
#include <string.h>

#include "dos_syntax.h"
#include "limits.h"
#include "startup_options.h"
#include "text_file.h"
#include "../lib/core/str.h"

static const char *const Keys[OptionCount] = { "BootMenu", "BootMenuDefault", "BootMenuDelay", "BootGUI",
                                               "Logo",     "BootKeys" };

const char *StartupOptions::key( int field ) {
  return Keys[field];
}

bool StartupOptions::isSwitch( int field ) {
  return field != OptMenuDefault && field != OptMenuDelay;
}

/* configValue() with a key in any case. */
static const char *valueOf( const char *line, int field ) {
  char key[20];
  strcpy( key, Keys[field] );
  upcaseStr( key );
  return configValue( line, key );
}

/* The [Options] block: its first line after the header and its end; false: none. */
static bool optionsBlock( const TextFile &msdos, int &first, int &end ) {
  char name[16];
  first = -1;
  for ( int i = 0; i < msdos.count(); i++ ) {
    if ( !sectionName( msdos.line( i ), name, sizeof( name ) ) ) continue;
    if ( first >= 0 ) {
      end = i;
      return true;
    }
    if ( cmpi( name, "OPTIONS" ) == 0 ) first = i + 1;
  }
  end = msdos.count();
  return first >= 0;
}

bool StartupOptions::isWindowsFile( const TextFile &msdos ) {
  char name[16];
  for ( int i = 0; i < msdos.count(); i++ ) {
    if ( sectionName( msdos.line( i ), name, sizeof( name ) ) &&
         ( cmpi( name, "OPTIONS" ) == 0 || cmpi( name, "PATHS" ) == 0 ) )
      return true;
  }
  return false;
}

StartupOptions::StartupOptions() {
  memset( values_, 0, sizeof( values_ ) );
}

void StartupOptions::read( const TextFile &msdos ) {
  memset( values_, 0, sizeof( values_ ) );
  int first, end;
  if ( !optionsBlock( msdos, first, end ) ) return;
  for ( int i = first; i < end; i++ ) {
    for ( int f = 0; f < OptionCount; f++ ) {
      const char *v = valueOf( msdos.line( i ), f );
      if ( v ) {
        copyStr( values_[f], v, sizeof( values_[f] ) );
        trim( values_[f] );
      }
    }
  }
}

int StartupOptions::invalidField() const {
  for ( int f = 0; f < OptionCount; f++ ) {
    const char *v = values_[f];
    for ( const char *s = v; *s; s++ ) {
      if ( *s < '0' || *s > '9' ) return f;
    }
    if ( isSwitch( f ) && *v && ( v[1] || ( *v != '0' && *v != '1' ) ) ) return f;
  }
  return -1;
}

static bool isKeyLine( const char *line ) {
  const char *s = skipBlank( line );
  return *s && *s != ';' && *s != '[' && strchr( s, '=' ) != NULL;
}

bool StartupOptions::write( TextFile &msdos ) const {
  int first, end;
  if ( !optionsBlock( msdos, first, end ) ) {
    if ( !msdos.insert( msdos.count(), "[Options]" ) ) return false;
    first = end = msdos.count();
  }
  bool ok = true;
  for ( int f = 0; f < OptionCount; f++ ) {
    int line = -1, lastKey = first - 1;
    for ( int i = first; i < end; i++ ) {
      if ( valueOf( msdos.line( i ), f ) ) line = i;
      if ( isKeyLine( msdos.line( i ) ) ) lastKey = i;
    }
    char text[MaxLine + 1], old[MaxValue + 2] = "";
    if ( line >= 0 ) {
      copyStr( old, valueOf( msdos.line( line ), f ), sizeof( old ) );
      trim( old );
    }
    if ( strcmp( old, values_[f] ) == 0 && ( line >= 0 || !values_[f][0] ) ) continue;
    strcpy( text, Keys[f] );
    strcat( text, "=" );
    strcat( text, values_[f] );
    if ( !values_[f][0] ) {
      msdos.remove( line );
      end--;
    }
    else if ( line >= 0 ) ok = msdos.replace( line, text ) && ok;
    else if ( msdos.insert( lastKey + 1, text ) ) end++;
    else ok = false;
  }
  return ok;
}
