/*
   Moving in a list: the arrows, PgUp / PgDn, Home / End.
*/

#ifndef BMAGIC_CONTROLLER_LIST_KEYS_H
#define BMAGIC_CONTROLLER_LIST_KEYS_H

class ListCursor;

/* false when the key does not move. */
bool moveInList( ListCursor &cursor, unsigned key );

#endif
