/*
   The message of the last failure. One Error is shared by the layers of a program: the
   layer that fails sets it, the caller that started the operation clears and shows it.
*/

#ifndef DOSLIB_CORE_ERROR_H
#define DOSLIB_CORE_ERROR_H

class Error {
public:
  enum { Size = 80 };

  Error();
  void set( const char *fmt, ... );
  void clear();
  bool isSet() const;
  const char *text() const;

private:
  char text_[Size];
};

/* Keeps the message of the first failure while cleaning up after it. */
class ErrorKeeper {
public:
  explicit ErrorKeeper( Error &error );
  ~ErrorKeeper();

private:
  Error &error_;
  char kept_[Error::Size];
};

#endif
