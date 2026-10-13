#include "boot_file.h"

static const char *const Names[BootFileCount] = { "CONFIG.SYS", "AUTOEXEC.BAT", "MSDOS.SYS" };

const char *bootFileName( int file ) {
  return Names[file];
}
