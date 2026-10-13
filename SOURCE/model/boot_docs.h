/*
   The boot files as BMAGIC holds them: each as it is on the disk and as it is being
   edited (two TextFiles that app/ allocates), and what the disk had.
*/

#ifndef BMAGIC_MODEL_BOOT_DOCS_H
#define BMAGIC_MODEL_BOOT_DOCS_H

#include "boot_file.h"

class TextFile;

enum FileState { FileMissing, FileText, FileBinary, FileTooBig, FileUnreadable };

struct BootDocs {
  TextFile *disk[BootFileCount];
  TextFile *edit[BootFileCount];
  char state[BootFileCount];      /* FileState */
  unsigned attrib[BootFileCount]; /* on the disk */

  /* A text file, or a missing one (that saving creates; not MSDOS.SYS). */
  bool isEditable( int file ) const {
    return state[file] == FileText || ( state[file] == FileMissing && file != MsdosSys );
  }
};

#endif
