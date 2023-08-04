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
		if (!isdigit(str)) {
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

SDL_Color HexToColour(uint32_t colour) {
	return (SDL_Color) {
		(colour >> 16) & 0xFF,
		(colour >> 8) & 0xFF,
		colour & 0xFF,
		255
	};
}
