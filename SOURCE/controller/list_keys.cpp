#include "list_keys.h"
#include "../lib/ui/keys.h"
#include "../lib/ui/list_cursor.h"

bool moveInList( ListCursor &cursor, unsigned key ) {
  if ( !isExtendedKey( key ) ) return false;
  int sel = cursor.sel(), page = cursor.rows() > 1 ? cursor.rows() - 1 : 1;
  switch ( keyScan( key ) ) {
    case ScanUp: sel--; break;
    case ScanDown: sel++; break;
    case ScanPgUp: sel -= page; break;
    case ScanPgDn: sel += page; break;
    case ScanHome: sel = 0; break;
    case ScanEnd: sel = cursor.count() - 1; break;
    default: return false;
  }
  if ( sel >= cursor.count() ) sel = cursor.count() - 1;
  if ( sel < 0 ) sel = 0;
  cursor.select( sel );
  return true;
}
