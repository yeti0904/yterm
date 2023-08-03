#ifndef YTERM_TEXTSCREEN_H
#define YTERM_TEXTSCREEN_H

#include "sdl.h"
#include "components.h"

typedef struct Vec2 {
	int x;
	int y;
} Vec2;

typedef struct Cell {
	char ch;
} Cell;

typedef struct TextScreen {
	Cell* cells;
	Vec2  size;
	Vec2  cursor;
} TextScreen;

TextScreen TextScreen_New(uint32_t w, uint32_t h);
void       TextScreen_Free(TextScreen* text);
Cell       TextScreen_GetCharacter(TextScreen* text, int x, int y);
void       TextScreen_SetCharacter(TextScreen* text, int x, int y, Cell cell);
void       TextScreen_PutCharacter(TextScreen* text, char ch);
void       TextScreen_PutString(TextScreen* text, char* str);
void       TextScreen_Render(TextScreen* text, Video* video);

#endif
