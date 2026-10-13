/*
   The backups page: the sets, newest first, with their date, note and files.
*/

#ifndef BMAGIC_VIEW_BACKUP_VIEW_H
#define BMAGIC_VIEW_BACKUP_VIEW_H

class BackupList;
class Frame;
class ListCursor;

class BackupView {
public:
  enum { FirstRow = 3, Rows = 18 };

  explicit BackupView( Frame &frame );
  /* folder: where the sets are */
  void draw( BackupList &sets, const ListCursor &cursor, const char *folder );

private:
  Frame &frame_;
};

#endif
