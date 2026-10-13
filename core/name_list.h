/*
   Names in a buffer, each terminated with a 0.
*/

#ifndef DOSLIB_CORE_NAME_LIST_H
#define DOSLIB_CORE_NAME_LIST_H

class NameList {
public:
  NameList( char *buf, unsigned size );
  char *buffer();
  unsigned size() const;
  /* After the buffer was filled from outside. */
  void setCount( int count );
  int count() const;
  /* false when it does not fit */
  bool add( const char *name );
  bool contains( const char *name ) const;
  void clear();

  /* for ( const char *n = list.first(); n; n = list.next( n ) ) */
  const char *first() const;
  const char *next( const char *name ) const;

private:
  char *buf_;
  unsigned size_;
  unsigned used_;
  int count_;
};

#endif
