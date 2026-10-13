#ifndef DOSLIB_PLATFORM_CONSOLE_BREAK_H
#define DOSLIB_PLATFORM_CONSOLE_BREAK_H

/* Ctrl+C and Ctrl+Break set a flag instead of ending the program. */
void consoleBreakCatch();
bool consoleBreakRaised();
void consoleBreakClear();

#endif
