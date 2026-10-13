/*
   The lines of CONFIG.SYS and AUTOEXEC.BAT as DOS reads them. Keywords and names compare
   case-insensitively; leading spaces and tabs do not count.
*/

#ifndef BMAGIC_MODEL_DOS_SYNTAX_H
#define BMAGIC_MODEL_DOS_SYNTAX_H

bool isBlankLine( const char *line );
/* "[NAME]": the name (trimmed, at most size - 1 characters). */
bool sectionName( const char *line, char *name, int size );
/* "KEY=value" or "KEY value" of CONFIG.SYS (key in upper case): where the value starts. */
const char *configValue( const char *line, const char *key );
/* ":LABEL" of a batch file (not a "::" comment): the label, up to the first blank. */
bool labelName( const char *line, char *name, int size );
/* Labels compare on their first 8 characters, as DOS jumps to them. */
bool sameLabel( const char *a, const char *b );
/* "GOTO %CONFIG%" (also "@GOTO", "GOTO :%CONFIG%"). */
bool isGotoConfig( const char *line );
/* "GOTO X": the label X. */
bool gotoTarget( const char *line, char *name, int size );
/* The part of a comma separated value before the first comma, trimmed; the rest after
   the comma (NULL when there is none). */
const char *splitComma( const char *value, char *first, int size );

#endif
