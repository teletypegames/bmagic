/*
   The main page: the items of the boot menu (their block, text, and how many lines they
   have in CONFIG.SYS and AUTOEXEC.BAT), the menu's settings, the state of the three files
   and of the Windows startup options, and the backups.
*/

#ifndef BMAGIC_VIEW_MENU_VIEW_H
#define BMAGIC_VIEW_MENU_VIEW_H

class BackupList;
class BootMenu;
class Frame;
struct BootDocs;

class MenuView {
public:
  enum { FirstRow = 3 };

  explicit MenuView( Frame &frame );
  /* changed: bit 1 << BootFile for a file with edits */
  void draw( const BootMenu &menu, const BootDocs &docs, unsigned changed, int sel, char drive, BackupList &sets );

private:
  void drawItems( const BootMenu &menu, const BootDocs &docs, int sel );
  void drawSettings( const BootMenu &menu, const BootDocs &docs );
  void drawFiles( const BootDocs &docs, unsigned changed, char drive );
  void drawBackups( BackupList &sets );

  Frame &frame_;
};

#endif
