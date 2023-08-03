#ifndef YTERM_SAFE_H
#define YTERM_SAFE_H

#include "components.h"

void* SafeMalloc(size_t size);
void* SafeCalloc(size_t size, size_t memberSize);
void* SafeRealloc(void* ptr, size_t size);

#endif
