#ifndef DOSLIB_CORE_PROGRESS_H
#define DOSLIB_CORE_PROGRESS_H

/* Where a long operation tells what it is doing. */
class Progress {
public:
  virtual void report( const char *text ) = 0;
};

/* printf into the progress (at most 159 characters). */
void reportf( Progress &progress, const char *fmt, ... );

#endif
