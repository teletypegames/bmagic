#include <string.h>

#include "fakes.h"
#include "../../lib/testing/host/fake_file_system.h"
#include "../../lib/testing/host/minitest.h"
#include "../../model/backup_list.h"
#include "../../model/boot_docs.h"
#include "../../model/text_file.h"
#include "../../repository/disk/drive_boot_file_repository.h"
#include "../../repository/disk/folder_backup_repository.h"
#include "../../service/backup_service.h"
#include "../../service/boot_service.h"

/* MSDOS.SYS is read-only, hidden and system on this disk. */
class AttribFileSystem : public FakeFileSystem {
public:
  bool find( const char *path, unsigned attribs, DirEntry &entry ) {
    if ( !FakeFileSystem::find( path, attribs, entry ) ) return false;
    if ( strstr( path, "MSDOS.SYS" ) ) entry.attrib = AttrReadOnly | AttrHidden | AttrSystem;
    return true;
  }
};

/* The services on a fake disk. */
struct Services {
  Services()
    : files( fs, 'C' ), backupFiles( fs, "C:\\APPS\\BMAGIC\\" ),
      backups( files, backupFiles, system, docs, sets, buf, 'C' ), boot( files, backups, system, docs, buf ) {
    fs.makeFolder( "C:\\APPS" );
    fs.makeFolder( "C:\\APPS\\BMAGIC" );
    for ( int f = 0; f < BootFileCount; f++ ) {
      docs.disk[f] = new TextFile;
      docs.edit[f] = new TextFile;
    }
    backup = new TextFile;
  }
  ~Services() {
    for ( int f = 0; f < BootFileCount; f++ ) {
      delete docs.disk[f];
      delete docs.edit[f];
    }
    delete backup;
  }

  AttribFileSystem fs;
  FakeBootSystem system;
  BootDocs docs;
  BackupList sets;
  char buf[TextFile::OutSize];
  TextFile *backup;
  DriveBootFileRepository files;
  FolderBackupRepository backupFiles;
  BackupService backups;
  BootService boot;
};

TEST( loadTellsTheStateOfEachFile ) {
  Services s;
  s.fs.add( "C:\\CONFIG.SYS", "FILES=30\r\n" );
  s.fs.addBytes( "C:\\MSDOS.SYS", "IO\0SYS", 6 );
  s.boot.load();
  CHECK_INT( s.docs.state[ConfigSys], FileText );
  CHECK_INT( s.docs.state[AutoexecBat], FileMissing );
  CHECK_INT( s.docs.state[MsdosSys], FileBinary );
  CHECK( s.docs.isEditable( AutoexecBat ) );
  CHECK( !s.docs.isEditable( MsdosSys ) );
  CHECK( !s.boot.isChanged() );
}

TEST( saveBacksUpThenWrites ) {
  Services s;
  s.fs.add( "C:\\CONFIG.SYS", "FILES=30\r\n" );
  s.fs.add( "C:\\MSDOS.SYS", "[Options]\r\nBootGUI=1\r\n" );
  s.boot.load();
  int set, failed;
  CHECK_INT( s.boot.save( set, failed ), BootService::Unchanged );
  s.docs.edit[ConfigSys]->replace( 0, "FILES=40" );
  s.docs.edit[AutoexecBat]->insert( 0, "@ECHO OFF" );
  s.docs.edit[MsdosSys]->replace( 1, "BootGUI=0" );
  CHECK( s.boot.isChanged() );
  CHECK_INT( s.boot.save( set, failed ), BootService::Saved );
  CHECK_INT( set, 1 );
  CHECK_STR( s.fs.data( "C:\\CONFIG.SYS" ), "FILES=40\r\n" );
  CHECK_STR( s.fs.data( "C:\\AUTOEXEC.BAT" ), "@ECHO OFF\r\n" );
  CHECK_STR( s.fs.data( "C:\\MSDOS.SYS" ), "[Options]\r\nBootGUI=0\r\n" );
  CHECK_STR( s.fs.data( "C:\\APPS\\BMAGIC\\BACKUP\\001\\CONFIG.SYS" ), "FILES=30\r\n" );
  CHECK( !s.fs.has( "C:\\APPS\\BMAGIC\\BACKUP\\001\\AUTOEXEC.BAT" ) );
  CHECK( s.fs.has( "C:\\APPS\\BMAGIC\\BACKUP\\001\\SET.INI" ) );
  CHECK( !s.fs.has( "C:\\CONFIG.$$$" ) && !s.fs.has( "C:\\CONFIG.BM$" ) );
  CHECK_INT( s.system.setCount, 1 );
  CHECK_STR( s.system.lastPath, "C:\\MSDOS.SYS" );
  CHECK_INT( s.system.lastAttrib, 7 );
  CHECK( !s.boot.isChanged() );
  CHECK_INT( s.sets.count(), 1 );
  CHECK_STR( s.sets.at( 0 ).date, "2026-10-13" );
  CHECK_STR( s.sets.at( 0 ).note, "Before saving" );
  CHECK( s.sets.at( 0 ).has( ConfigSys ) && s.sets.at( 0 ).has( MsdosSys ) && !s.sets.at( 0 ).has( AutoexecBat ) );
}

TEST( restoreBacksUpFirst ) {
  Services s;
  s.fs.add( "C:\\CONFIG.SYS", "OLD\r\n" );
  s.boot.load();
  CHECK_INT( s.backups.backup( "Made by hand" ), 1 );
  s.fs.add( "C:\\CONFIG.SYS", "NEW\r\n" );
  int made;
  CHECK( s.backups.restore( 1, made ) );
  CHECK_INT( made, 2 );
  CHECK_STR( s.fs.data( "C:\\CONFIG.SYS" ), "OLD\r\n" );
  CHECK_STR( s.fs.data( "C:\\APPS\\BMAGIC\\BACKUP\\002\\CONFIG.SYS" ), "NEW\r\n" );
  CHECK_STR( s.sets.at( 0 ).note, "Before restoring 001" );
  s.boot.load();
  CHECK( s.backups.load( 2, ConfigSys, *s.backup ) );
  CHECK_STR( s.backup->line( 0 ), "NEW" );
  CHECK( s.backups.load( 2, AutoexecBat, *s.backup ) );
  CHECK_INT( s.backup->count(), 0 );
  CHECK( s.backups.remove( 1 ) );
  CHECK_INT( s.sets.count(), 1 );
  CHECK( !s.fs.exists( "C:\\APPS\\BMAGIC\\BACKUP\\001" ) );
}

TEST( writeKeepsTheOldFileOnAFailure ) {
  Services s;
  s.fs.add( "C:\\CONFIG.SYS", "OLD\r\n" );
  s.fs.add( "C:\\CONFIG.$$$", "" );
  s.fs.lock( "C:\\CONFIG.$$$" );
  CHECK( !s.files.write( ConfigSys, "NEW\r\n", 5 ) );
  CHECK_STR( s.fs.data( "C:\\CONFIG.SYS" ), "OLD\r\n" );
}

TEST( writeReplacesAStaleOldFile ) {
  Services s;
  s.fs.add( "C:\\CONFIG.SYS", "OLD\r\n" );
  s.fs.add( "C:\\CONFIG.BM$", "stale" );
  CHECK( s.files.write( ConfigSys, "NEW\r\n", 5 ) );
  CHECK_STR( s.fs.data( "C:\\CONFIG.SYS" ), "NEW\r\n" );
  CHECK( !s.fs.has( "C:\\CONFIG.BM$" ) && !s.fs.has( "C:\\CONFIG.$$$" ) );
}
