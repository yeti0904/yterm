#ifndef YTERM_UTIL_H
#define YTERM_UTIL_H

#include "components.h"

#define ARRAY_LEN(x) (sizeof(x) / sizeof(*(x)))
#define FATAL(x)     {fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, x); exit(1);}

void      DumpSequence(char* seq);
bool      StringStartsWith(char* str, char* with);
bool      StringIsNumeric(char* str);
char*     DupString(char* str);
SDL_Color HexToColour(uint32_t colour);

#endif
