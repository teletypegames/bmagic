/*
   A small test framework for the unit tests on the host:

     TEST( name ) { CHECK( 1 + 1 == 2 ); CHECK_STR( "a", "a" ); }

   runTests() runs every TEST of the program and returns the exit code.
*/

#ifndef DOSLIB_TESTING_MINITEST_H
#define DOSLIB_TESTING_MINITEST_H

typedef void ( *TestFunction )();

class TestRegistration {
public:
  TestRegistration( const char *name, TestFunction function );
};

void checkTrue( bool ok, const char *expr, const char *file, int line );
void checkStr( const char *got, const char *want, const char *expr, const char *file, int line );
void checkInt( long got, long want, const char *expr, const char *file, int line );
int runTests( int argc, char *argv[] );

#define TEST( name )                                                                                                   \
  static void name();                                                                                                  \
  static TestRegistration name##Registration( #name, name );                                                           \
  static void name()

#define CHECK( expr ) checkTrue( ( expr ), #expr, __FILE__, __LINE__ )
#define CHECK_STR( got, want ) checkStr( ( got ), ( want ), #got, __FILE__, __LINE__ )
#define CHECK_INT( got, want ) checkInt( ( got ), ( want ), #got, __FILE__, __LINE__ )

#endif
