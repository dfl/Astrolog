/*
** Stub implementations for symbols defined in astrolog.cpp that are
** referenced by other translation units.  These are never exercised
** during unit testing but must be present to satisfy the linker.
*/

#include "astrolog.h"

void Action(void) {}

flag FProcessCommandLine(CONST char *szLine) { return fFalse; }

int NParseCommandLine(char *szLine, char **argv) { return 0; }

int NPromptSwitches(char *line, char *argv[MAXSWITCHES]) { return 0; }

flag FProcessSwitches(int argc, char **argv) { return fFalse; }

void FinalizeProgram(flag fSkip) {}
