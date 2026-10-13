/*
   The boot menu as CONFIG.SYS and AUTOEXEC.BAT describe it (MS-DOS 6 and later):

     CONFIG.SYS                       AUTOEXEC.BAT
       [MENU]                           ...the lines of every configuration...
       MENUITEM=NORMAL,Normal start     GOTO %CONFIG%
       MENUITEM=GAMES,Games (EMS)       :NORMAL
       MENUDEFAULT=NORMAL,10            ...
       MENUCOLOR=15,1                   GOTO END
       NUMLOCK=OFF                      :GAMES
       [COMMON]                         ...
       ...                              GOTO END
       [NORMAL]                         :END
       DEVICE=...
       [GAMES]
       ...

   An item is a MENUITEM line of the [MENU] block, its [NAME] block in CONFIG.SYS and its
   :NAME section in AUTOEXEC.BAT (from the label to the next label). The sections end
   with a GOTO to a common label (the end label, :END by default). scan() maps the lines;
   the line numbers are valid until the files change. Only the first 9 MENUITEM lines
   count (DOS shows at most 9); SUBMENU lines are kept but not mapped.
*/

#ifndef BMAGIC_MODEL_BOOT_MENU_H
#define BMAGIC_MODEL_BOOT_MENU_H

#include "limits.h"

class TextFile;

struct MenuItem {
  char name[MaxBlock + 1];
  char text[MaxMenuText + 1]; /* empty: DOS shows the name */
  int itemLine;               /* the MENUITEM line in CONFIG.SYS */
  int blockLine, blockEnd;    /* the [NAME] line and the line after its block; -1: none */
  int labelLine, labelEnd;    /* the :NAME line in AUTOEXEC.BAT and the line after its
                                 section; -1: none */
};

class BootMenu {
public:
  enum { MaxItems = 9, NoTimeout = -1 };

  BootMenu();
  void scan( const TextFile &config, const TextFile &autoexec );
  bool hasMenu() const { return menuLine >= 0; }
  /* The item of a block name, or -1. */
  int find( const char *name ) const;
  /* The MENUDEFAULT item, or -1. */
  int defaultItem() const;
  /* GOTO %CONFIG% jumps to the item's label: false when the label is missing. */
  bool labelMissing( int item ) const;
  /* The first [NAME] block of CONFIG.SYS (also outside the menu), or -1; its end in end. */
  static int findBlock( const TextFile &config, const char *name, int &end );
  /* The lines of a block or section after its header, without the blank lines at its
     end; -1 when header is -1. */
  static int bodyLines( const TextFile &text, int header, int end );
  /* The first :LABEL of a batch file, or -1. */
  static int findLabel( const TextFile &batch, const char *name );

  int count;
  MenuItem items[MaxItems];
  int lastItemLine; /* the last MENUITEM line, also over the 9 (-1: none) */
  int extraItems;   /* MENUITEM lines over the 9 */
  int submenus;     /* SUBMENU lines */
  int menuLine;     /* [MENU], -1: no menu */
  int menuEnd;
  int defaultLine; /* MENUDEFAULT, -1 */
  char defaultName[MaxBlock + 1];
  int timeout; /* seconds, NoTimeout: the menu waits */
  int colorLine;
  char color[16];
  int numlockLine;
  char numlock[8];
  int gotoLine;     /* GOTO %CONFIG% in AUTOEXEC.BAT, -1 */
  int endLabelLine; /* the label the sections jump to at their end, -1 */
  char endLabel[MaxBlock + 1];

private:
  void reset();
  void scanMenu( const TextFile &config );
  void scanMenuLine( const char *line, int i );
  void scanAutoexec( const TextFile &autoexec );
};

#endif
