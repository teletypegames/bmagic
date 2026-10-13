/*
   The diff pages: the edits not saved yet (each file against the disk; as the review
   before saving, F2 saves), or a backup against the files now on the disk. Tab goes to
   the next file that differs; the arrows and paging scroll, Left / Right shift the
   lines, Esc goes back.
*/

#ifndef BMAGIC_CONTROLLER_DIFF_CONTROLLER_H
#define BMAGIC_CONTROLLER_DIFF_CONTROLLER_H

class BackupService;
class DiffView;
class Frame;
class Keyboard;
class TextDiff;
class TextFile;
struct BootDocs;

class DiffController {
public:
  enum Result { NoChanges, Back, Confirmed };

  DiffController( DiffView &view, Frame &frame, Keyboard &keyboard, TextDiff &diff, BootDocs &docs,
                  BackupService &backups, TextFile &backup );
  /* confirm: the review before saving (F2: Confirmed) */
  Result showEdits( bool confirm );
  Result showBackup( int set );

private:
  /* The two versions of a file; false when it has none to compare. */
  bool pair( int file, const TextFile *&before, const TextFile *&after );
  Result run( const char *title, bool confirm );

  DiffView &view_;
  Frame &frame_;
  Keyboard &keyboard_;
  TextDiff &diff_;
  BootDocs &docs_;
  BackupService &backups_;
  TextFile &backup_;
  int set_; /* 0: the edits */
};

#endif
