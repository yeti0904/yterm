#include "safe.h"
#include "util.h"

void* SafeMalloc(size_t size) {
	void* ret = malloc(size);

	if (ret == NULL) {
		FATAL("Malloc failed");
	}

	return ret;
}

void* SafeCalloc(size_t size, size_t memberSize) {
	void* ret = calloc(size, memberSize);

	if (ret == NULL) {
		FATAL("Calloc failed");
	}

	return ret;
}

void* SafeRealloc(void* ptr, size_t size) {
	void* ret = realloc(ptr, size);

	if (ret == NULL) {
		FATAL("Realloc failed");
	}

	return ret;
}
