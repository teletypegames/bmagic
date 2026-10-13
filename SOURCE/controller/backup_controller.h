/*
   The backups page: Enter compares a backup with the files now on the disk, Ins backs
   them up now, R restores a backup (after a Y; the files it replaces are backed up first,
   the edits not saved are dropped), Del deletes one (after a Y), Esc goes back.
*/

#ifndef BMAGIC_CONTROLLER_BACKUP_CONTROLLER_H
#define BMAGIC_CONTROLLER_BACKUP_CONTROLLER_H

#include "../lib/ui/list_cursor.h"

class BackupList;
class BackupService;
class BackupView;
class BootService;
class DiffController;
class Frame;
class Keyboard;

class BackupController {
public:
  BackupController( BackupView &view, Frame &frame, Keyboard &keyboard, BackupService &backups, BootService &boot,
                    BackupList &sets, DiffController &diffs, const char *folder );
  /* true when a backup was restored (the files were loaded again). */
  bool run();

private:
  bool ask( const char *question, int set );
  void compare();
  void backupNow();
  bool restore();
  void remove();

  BackupView &view_;
  Frame &frame_;
  Keyboard &keyboard_;
  BackupService &backups_;
  BootService &boot_;
  BackupList &sets_;
  DiffController &diffs_;
  const char *folder_;
  ListCursor cursor_;
  const char *message_;
  bool error_;
  char text_[80];
};

#endif
