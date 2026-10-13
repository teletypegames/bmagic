#include <stdio.h>
#include <string.h>

#include "minitest.h"

struct Test {
  const char *name;
  TestFunction function;
};

static Test Tests[512];
static int TestCount = 0;
static int Failures = 0;
static const char *Current = "";

TestRegistration::TestRegistration( const char *name, TestFunction function ) {
  Tests[TestCount].name = name;
  Tests[TestCount].function = function;
  TestCount++;
}

static void fail( const char *file, int line ) {
  Failures++;
  printf( "FAIL %s (%s:%d): ", Current, file, line );
}

void checkTrue( bool ok, const char *expr, const char *file, int line ) {
  if ( ok ) return;
  fail( file, line );
  printf( "%s\n", expr );
}

void checkStr( const char *got, const char *want, const char *expr, const char *file, int line ) {
  if ( strcmp( got, want ) == 0 ) return;
  fail( file, line );
  printf( "%s\n  got:  \"%s\"\n  want: \"%s\"\n", expr, got, want );
}

void checkInt( long got, long want, const char *expr, const char *file, int line ) {
  if ( got == want ) return;
  fail( file, line );
  printf( "%s\n  got:  %ld\n  want: %ld\n", expr, got, want );
}

int runTests( int argc, char *argv[] ) {
  int run = 0;
  for ( int i = 0; i < TestCount; i++ ) {
    if ( argc > 1 && strstr( Tests[i].name, argv[1] ) == NULL ) continue;
    Current = Tests[i].name;
    Tests[i].function();
    run++;
  }
  printf( "%d tests, %d failures\n", run, Failures );
  return Failures ? 1 : 0;
}
