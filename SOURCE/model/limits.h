/*
   The sizes BMAGIC works with.
*/

#ifndef BMAGIC_MODEL_LIMITS_H
#define BMAGIC_MODEL_LIMITS_H

enum {
  MaxFileSize = 16000, /* a boot file larger than this is not edited */
  MaxLine = 200,       /* an edited line (longer lines of the files are kept as they are) */
  MaxBlock = 70,       /* a block name as DOS takes it */
  MaxNewBlock = 8,     /* a new block name: AUTOEXEC.BAT labels count 8 characters */
  MaxMenuText = 70,
  PathSize = 80
};

#endif
