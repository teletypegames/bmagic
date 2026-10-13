/*
   Changes the boot menu in CONFIG.SYS and AUTOEXEC.BAT (see BootMenu), line by line: the
   lines it does not touch stay as they are. Every change maps the files again.

   - add: a MENUITEM line after the last one, a [NAME] block after the last block of the
     items, a :NAME section before the end label that ends with GOTO END. The first item
     of a CONFIG.SYS without a menu starts it: [MENU] and the item go to the top, and the
     lines that were there go under a [COMMON] header. An AUTOEXEC.BAT without
     GOTO %CONFIG% gets it at its end, with a section for every item and the end label.
     A copy gets the lines of another item's block and section.
   - rename: the MENUITEM line, the [NAME] line, MENUDEFAULT, INCLUDE=NAME lines and the
     :NAME label.
   - remove: the MENUITEM line, the block, the section, MENUDEFAULT and the INCLUDE lines
     of the item; the last item takes the [MENU] block and GOTO %CONFIG% with it.

   A new block name is 1-8 letters, digits, '_' or '-' (upper-cased), not MENU or
   COMMON, and differs from the other items in its first 8 characters (the labels).
*/

#ifndef BMAGIC_MODEL_MENU_EDITOR_H
#define BMAGIC_MODEL_MENU_EDITOR_H

class BootMenu;
class TextFile;

class MenuEditor {
public:
  enum Result { Done, BadName, NameTaken, MenuFull, NoMenu, BadTimeout, BadColor, BadNumlock, FileFull };

  MenuEditor( TextFile &config, TextFile &autoexec, BootMenu &menu );
  void rescan();
  /* name is upper-cased in place. */
  Result checkName( char *name, int except ) const;
  /* copyOf: the item whose lines the new one gets, or -1. */
  Result add( char *name, const char *text, int copyOf );
  Result rename( int item, char *name );
  Result setText( int item, const char *text );
  Result remove( int item );
  /* Swaps the item with the one delta places away. */
  Result move( int item, int delta );
  /* item -1: no MENUDEFAULT; timeout -1: the menu waits. */
  Result setDefault( int item, int timeout );
  /* "15,1" (text, background), or empty for none. */
  Result setColor( const char *color );
  /* "ON", "OFF", or empty for none. */
  Result setNumlock( const char *numlock );
  /* The item's [NAME] block / :NAME section, made when it is missing. */
  Result ensureBlock( int item );
  Result ensureSection( int item );

private:
  bool insertBlock( const char *name, int copyOf );
  bool insertSection( const char *name, int copyOf );
  bool addDispatcher( int copyOf, int item );
  void removeLines( TextFile &file, int from, int to );
  void removeIncludes( const char *name );
  int optionLine() const;
  bool setMenuLine( int &line, const char *key, const char *value );

  TextFile &config_;
  TextFile &autoexec_;
  BootMenu &menu_;
};

/* What a Result means, for the user. */
const char *menuEditMessage( MenuEditor::Result result );

#endif
