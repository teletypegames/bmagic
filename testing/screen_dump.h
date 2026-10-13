#ifndef DOSLIB_TESTING_SCREEN_DUMP_H
#define DOSLIB_TESTING_SCREEN_DUMP_H

class FileSystem;
class Screen;

/* The screen as 80x25 character / attribute pairs (SCREEN.BIN of the tests). */
bool dumpScreen( Screen &screen, FileSystem &fs, const char *path );

#endif
