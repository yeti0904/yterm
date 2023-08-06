#ifndef YTERM_UTIL_H
#define YTERM_UTIL_H

#include "components.h"

#define ARRAY_LEN(x) (sizeof(x) / sizeof(*(x)))
#define FATAL(...) \
	( \
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__), \
		fprintf(stderr, __VA_ARGS__), \
		fprintf(stderr, "\n"), \
	 	exit(EXIT_FAILURE) \
	)

void  DumpSequence(char* seq);
bool  StringStartsWith(char* str, char* with);
bool  StringIsNumeric(char* str);
char* DupString(char* str);
bool  HexToColour(const char *colour, SDL_Color* ret);

#endif
