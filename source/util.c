#include "components.h"
#include "safe.h"

void DumpSequence(char* seq) {
	for (size_t i = 0; i < strlen(seq); ++ i) {
		switch (seq[i]) {
			case '\x1b': {
				fputs("\\e", stdout);
				break;
			}
			default: {
				putchar(seq[i]);
			}
		}
	}
	putchar('\n');
}

bool StringStartsWith(char* str, char* with) {
	if (strlen(with) > strlen(str)) {
		return false;
	}

	return memcmp(str, with, strlen(with)) == 0;
}

bool StringIsNumeric(char* str) {
	for (size_t i = 0; i < strlen(str); ++ i) {
		if (!isdigit(str[i])) {
			return false;
		}
	}

	return true;
}

char* DupString(char* str) {
	char* ret = SafeMalloc(strlen(str) + 1);
	strcpy(ret, str);
	return ret;
}

bool HexToColour(const char* colour, SDL_Color* ret) {
	assert(ret != NULL);

	char* ptr;
	uint32_t colour32 = (uint32_t) strtol(colour, &ptr, 16);
	if (*ptr != '\0')
		return false;

	ret->r = (colour32 >> 16) & 0xFF;
	ret->g = (colour32 >> 8)  & 0xFF;
	ret->b = colour32         & 0xFF;
	ret->a = 255;

	return true;
}
