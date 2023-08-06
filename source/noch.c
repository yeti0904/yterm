#include "safe.h"

#define NOCH_ALLOC(SIZE)        SafeMalloc (SIZE)
#define NOCH_REALLOC(PTR, SIZE) SafeRealloc(PTR, SIZE)
#define NOCH_FREE(PTR)          free       (PTR)

#include <noch/json.c>
#include <noch/args.c>
