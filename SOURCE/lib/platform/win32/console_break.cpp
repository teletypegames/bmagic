#include <windows.h>

#include "console_break.h"

static volatile bool Raised = false;

static BOOL WINAPI handler( DWORD type ) {
  if ( type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT ) {
    Raised = true;
    return TRUE;
  }
  return FALSE;
}

void consoleBreakCatch() {
  SetConsoleCtrlHandler( handler, TRUE );
}

bool consoleBreakRaised() {
  return Raised;
}

void consoleBreakClear() {
  Raised = false;
}
