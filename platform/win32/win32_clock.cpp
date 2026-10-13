#include <windows.h>

#include "win32_clock.h"

unsigned long Win32Clock::ms() {
  return GetTickCount();
}

unsigned Win32Clock::ticks() {
  return (unsigned)( GetTickCount() / 55 );
}

unsigned Win32Clock::today() {
  SYSTEMTIME t;
  GetLocalTime( &t );
  return ( ( t.wYear - 1980 ) << 9 ) | ( t.wMonth << 5 ) | t.wDay;
}
