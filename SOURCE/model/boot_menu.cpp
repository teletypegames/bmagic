#include <stdlib.h>
#include <string.h>

#include "boot_menu.h"
#include "dos_syntax.h"
#include "text_file.h"
#include "../lib/core/str.h"

BootMenu::BootMenu() {
  reset();
}

void BootMenu::reset() {
  memset( this, 0, sizeof( *this ) );
  menuLine = gotoLine = endLabelLine = lastItemLine = defaultLine = colorLine = numlockLine = -1;
}

static int sectionEnd( const TextFile &config, int header ) {
  char name[MaxBlock + 1];
  int i = header + 1;
  while ( i < config.count() && !sectionName( config.line( i ), name, sizeof( name ) ) ) i++;
  return i;
}

int BootMenu::findBlock( const TextFile &config, const char *name, int &end ) {
  char section[MaxBlock + 1];
  for ( int i = 0; i < config.count(); i++ ) {
    if ( sectionName( config.line( i ), section, sizeof( section ) ) && cmpi( section, name ) == 0 ) {
      end = sectionEnd( config, i );
      return i;
    }
  }
  end = -1;
  return -1;
}

int BootMenu::bodyLines( const TextFile &text, int header, int end ) {
  if ( header < 0 ) return -1;
  while ( end > header + 1 && isBlankLine( text.line( end - 1 ) ) ) end--;
  return end - header - 1;
}

int BootMenu::findLabel( const TextFile &batch, const char *name ) {
  char label[MaxBlock + 1];
  for ( int i = 0; i < batch.count(); i++ ) {
    if ( labelName( batch.line( i ), label, sizeof( label ) ) && sameLabel( label, name ) ) return i;
  }
  return -1;
}

void BootMenu::scan( const TextFile &config, const TextFile &autoexec ) {
  reset();
  menuLine = findBlock( config, "MENU", menuEnd );
  if ( menuLine >= 0 ) scanMenu( config );
  for ( int i = 0; i < count; i++ ) {
    items[i].blockLine = findBlock( config, items[i].name, items[i].blockEnd );
  }
  scanAutoexec( autoexec );
}

void BootMenu::scanMenuLine( const char *line, int i ) {
  const char *v;
  if ( ( v = configValue( line, "MENUITEM" ) ) != NULL ) {
    lastItemLine = i;
    if ( count == MaxItems ) {
      extraItems++;
      return;
    }
    MenuItem &item = items[count++];
    const char *text = splitComma( v, item.name, sizeof( item.name ) );
    if ( text ) {
      copyStr( item.text, skipBlank( text ), sizeof( item.text ) );
      trim( item.text );
    }
    item.itemLine = i;
  }
  else if ( ( v = configValue( line, "MENUDEFAULT" ) ) != NULL ) {
    defaultLine = i;
    const char *t = splitComma( v, defaultName, sizeof( defaultName ) );
    t = t ? skipBlank( t ) : NULL;
    timeout = ( t && *t >= '0' && *t <= '9' ) ? atoi( t ) : NoTimeout;
  }
  else if ( ( v = configValue( line, "MENUCOLOR" ) ) != NULL ) {
    colorLine = i;
    copyStr( color, v, sizeof( color ) );
    trim( color );
  }
  else if ( ( v = configValue( line, "NUMLOCK" ) ) != NULL ) {
    numlockLine = i;
    copyStr( numlock, v, sizeof( numlock ) );
    trim( numlock );
  }
  else if ( configValue( line, "SUBMENU" ) ) submenus++;
}

void BootMenu::scanMenu( const TextFile &config ) {
  timeout = NoTimeout;
  for ( int i = menuLine + 1; i < menuEnd; i++ ) scanMenuLine( config.line( i ), i );
}

void BootMenu::scanAutoexec( const TextFile &autoexec ) {
  char target[MaxBlock + 1];
  for ( int i = 0; i < autoexec.count() && gotoLine < 0; i++ ) {
    if ( isGotoConfig( autoexec.line( i ) ) ) gotoLine = i;
  }
  for ( int k = 0; k < count; k++ ) {
    MenuItem &item = items[k];
    item.labelLine = findLabel( autoexec, item.name );
    item.labelEnd = -1;
    if ( item.labelLine < 0 ) continue;
    int i = item.labelLine + 1;
    while ( i < autoexec.count() && !labelName( autoexec.line( i ), target, sizeof( target ) ) ) {
      if ( !endLabel[0] && gotoTarget( autoexec.line( i ), target, sizeof( target ) ) &&
           cmpi( target, "%CONFIG%" ) != 0 )
        strcpy( endLabel, target );
      i++;
    }
    item.labelEnd = i;
  }
  if ( !endLabel[0] ) strcpy( endLabel, "END" );
  endLabelLine = findLabel( autoexec, endLabel );
}

int BootMenu::find( const char *name ) const {
  for ( int i = 0; i < count; i++ ) {
    if ( cmpi( items[i].name, name ) == 0 ) return i;
  }
  return -1;
}

int BootMenu::defaultItem() const {
  return defaultLine >= 0 ? find( defaultName ) : -1;
}

bool BootMenu::labelMissing( int item ) const {
  return gotoLine >= 0 && items[item].labelLine < 0;
}
