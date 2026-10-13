/*
   The three files of the boot menu in the root folder of the boot drive.
*/

#ifndef BMAGIC_MODEL_BOOT_FILE_H
#define BMAGIC_MODEL_BOOT_FILE_H

enum BootFile { ConfigSys, AutoexecBat, MsdosSys, BootFileCount };

/* "CONFIG.SYS", "AUTOEXEC.BAT", "MSDOS.SYS" */
const char *bootFileName( int file );

#endif
