/*
   The main page (see MenuView and the keys in HelpView): the items of the boot menu, and
   the way to the other pages. Saving goes through the review of the changes (F2); Esc
   with changes that are not saved asks first. The result is the exit code: 0 when the
   files were written (saved or restored), 1 otherwise.
*/

#ifndef BMAGIC_CONTROLLER_MENU_CONTROLLER_H
#define BMAGIC_CONTROLLER_MENU_CONTROLLER_H

#include "../lib/ui/list_cursor.h"

class BackupController;
class BackupList;
class BootMenu;
class BootService;
class DiffController;
class FormController;
class Frame;
class HelpController;
class Keyboard;
class LinesController;
class MenuEditor;
class MenuView;
class Speaker;
struct BootDocs;

class MenuController {
public:
  MenuController( MenuView &view, Frame &frame, Keyboard &keyboard, Speaker &speaker, BootDocs &docs, BootMenu &menu,
                  MenuEditor &editor, BootService &boot, BackupList &sets, FormController &form, LinesController &lines,
                  DiffController &diffs, BackupController &backups, HelpController &help, char drive );
  int run();

private:
  bool handleKey( unsigned key );
  bool handleFunctionKey( unsigned char scan );
  bool quit();
  void save();
  bool canEdit();
  bool hasItem();
  void newItem();
  void editItem();
  void deleteItem();
  void moveItem( int delta );
  void makeDefault();
  void menuOptions();
  void startupOptions();
  void report( int result );
  void say( const char *text, bool error );

  MenuView &view_;
  Frame &frame_;
  Keyboard &keyboard_;
  Speaker &speaker_;
  BootDocs &docs_;
  BootMenu &menu_;
  MenuEditor &editor_;
  BootService &boot_;
  BackupList &sets_;
  FormController &form_;
  LinesController &lines_;
  DiffController &diffs_;
  BackupController &backups_;
  HelpController &help_;
  char drive_;
  ListCursor cursor_;
  const char *message_;
  bool error_;
  bool written_;
  char text_[100];
};

#endif
