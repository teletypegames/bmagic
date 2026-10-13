#include <stddef.h>

#include "bmagic.h"
#include "version.h"
#include "../controller/backup_controller.h"
#include "../controller/diff_controller.h"
#include "../controller/form_controller.h"
#include "../controller/help_controller.h"
#include "../controller/line_input.h"
#include "../controller/lines_controller.h"
#include "../controller/menu_controller.h"
#include "../lib/core/str.h"
#include "../lib/platform/keyboard.h"
#include "../lib/platform/screen.h"
#include "../lib/platform/text_output.h"
#include "../lib/ui/canvas.h"
#include "../model/backup_list.h"
#include "../model/boot_docs.h"
#include "../model/boot_menu.h"
#include "../model/limits.h"
#include "../model/line_rows.h"
#include "../model/menu_editor.h"
#include "../model/text_diff.h"
#include "../model/text_file.h"
#include "../platform/boot_system.h"
#include "../repository/disk/drive_boot_file_repository.h"
#include "../repository/disk/folder_backup_repository.h"
#include "../service/backup_service.h"
#include "../service/boot_service.h"
#include "../view/backup_view.h"
#include "../view/diff_view.h"
#include "../view/form_view.h"
#include "../view/frame.h"
#include "../view/help_view.h"
#include "../view/lines_view.h"
#include "../view/menu_view.h"

/* Boot Magic put together: the members are built in this order. The TextFiles (17 KB
   each) are allocated one by one, a 16-bit object is at most 64 KB. */
struct Bmagic {
  Bmagic( Machine &m, BootDocs &docs, TextFile &backup, const char *progDir, char drive );

  BootMenu menu;
  BackupList sets;
  LineRows rows;
  TextDiff diff;
  char buf[TextFile::OutSize];
  char folder[PathSize];
  MenuEditor editor;
  DriveBootFileRepository files;
  FolderBackupRepository backupFiles;
  BackupService backups;
  BootService boot;
  Canvas canvas;
  Frame frame;
  MenuView menuView;
  LinesView linesView;
  DiffView diffView;
  BackupView backupView;
  FormView formView;
  HelpView helpView;
  LineInput input;
  FormController form;
  HelpController help;
  DiffController diffs;
  LinesController lines;
  BackupController backupPage;
  MenuController main;
};

Bmagic::Bmagic( Machine &m, BootDocs &docs, TextFile &backup, const char *progDir, char drive )
  : editor( *docs.edit[ConfigSys], *docs.edit[AutoexecBat], menu ), files( m.fs, drive ), backupFiles( m.fs, progDir ),
    backups( files, backupFiles, m.system, docs, sets, buf, drive ), boot( files, backups, m.system, docs, buf ),
    canvas( m.screen ), frame( canvas ), menuView( frame ), linesView( frame ), diffView( frame ), backupView( frame ),
    formView( frame ), helpView( frame ), input( m.speaker ), form( formView, m.keyboard, input ),
    help( helpView, frame, m.keyboard ), diffs( diffView, frame, m.keyboard, diff, docs, backups, backup ),
    lines( linesView, frame, m.keyboard, m.speaker, input, docs, menu, editor, rows, boot, diffs, help, drive ),
    backupPage( backupView, frame, m.keyboard, backups, boot, sets, diffs, folder ),
    main( menuView, frame, m.keyboard, m.speaker, docs, menu, editor, boot, sets, form, lines, diffs, backupPage, help,
          drive ) {
  backupFiles.folder( 0, folder );
}

/* The TextFiles of the documents and the backup. */
struct Texts {
  TextFile *all[2 * BootFileCount + 1];

  bool allocate() {
    bool ok = true;
    for ( int i = 0; i < 2 * BootFileCount + 1; i++ ) {
      all[i] = new TextFile;
      ok = ok && all[i] != NULL;
    }
    return ok;
  }
  void release() {
    for ( int i = 0; i < 2 * BootFileCount + 1; i++ ) delete all[i];
  }
};

static const char Usage[] = " - edits the boot menu of CONFIG.SYS and AUTOEXEC.BAT\r\n"
                            "Usage: BMAGIC [X:]   (X: a drive other than the boot drive)\r\n";

/* The drive of the arguments ("D:"), or 0 when there is none; false on a bad argument. */
static bool parseArgs( int argc, char *argv[], char &drive ) {
  drive = 0;
  for ( int i = 1; i < argc; i++ ) {
    char d = upcase( argv[i][0] );
    if ( d >= 'A' && d <= 'Z' && argv[i][1] == ':' && ( argv[i][2] == 0 || argv[i][2] == '\\' ) ) drive = d;
    else return false;
  }
  return true;
}

static int edit( Machine &m, Bmagic &app ) {
  app.boot.load();
  app.backups.refresh();
  m.screen.begin();
  m.keyboard.begin();
  int code = app.main.run();
  m.keyboard.end();
  m.screen.end();
  if ( code == 0 ) m.out.write( "Boot Magic: restart the computer to use the new boot files.\r\n" );
  return code;
}

int runBmagic( Machine &m, const char *progDir, int argc, char *argv[] ) {
  char drive;
  if ( !parseArgs( argc, argv, drive ) ) {
    m.out.write( "Boot Magic " );
    m.out.write( Version );
    m.out.write( Usage );
    return 2;
  }
  if ( drive == 0 ) drive = m.system.bootDrive();
  Texts texts;
  BootDocs docs;
  Bmagic *app = NULL;
  if ( texts.allocate() ) {
    for ( int f = 0; f < BootFileCount; f++ ) {
      docs.disk[f] = texts.all[2 * f];
      docs.edit[f] = texts.all[2 * f + 1];
      docs.state[f] = FileMissing;
      docs.attrib[f] = 0;
    }
    app = new Bmagic( m, docs, *texts.all[2 * BootFileCount], progDir, drive );
  }
  int code = 2;
  if ( app == NULL ) m.out.write( "Boot Magic: not enough memory.\r\n" );
  else code = edit( m, *app );
  delete app;
  texts.release();
  return code;
}
