#ifndef YTERM_TEXTSCREEN_H
#define YTERM_TEXTSCREEN_H

#include "sdl.h"
#include "components.h"

enum { // colours
	COLOUR_BLACK = 0,
	COLOUR_RED,
	COLOUR_GREEN,
	COLOUR_YELLOW,
	COLOUR_BLUE,
	COLOUR_MAGENTA,
	COLOUR_CYAN,
	COLOUR_WHITE,
	COLOUR_GREY,
	COLOUR_BRIGHT_RED,
	COLOUR_BRIGHT_GREEN,
	COLOUR_BRIGHT_YELLOW,
	COLOUR_BRIGHT_BLUE,
	COLOUR_BRIGHT_MAGENTA,
	COLOUR_BRIGHT_CYAN,
	COLOUR_BRIGHT_WHITE
};

enum { // attributes
	ATTR_NONE      = 0,
	ATTR_COLOUR_FG = 1,
	ATTR_COLOUR_BG = 2,
	ATTR_BOLD      = 4,
	ATTR_DIM       = 8,
	ATTR_ITALIC    = 16,
	ATTR_UNDERLINE = 32,
	ATTR_BLINKING  = 64,
	ATTR_REVERSE   = 128,
	ATTR_HIDDEN    = 256,
	ATTR_STRIKE    = 512
};

typedef struct ColourScheme {
	SDL_Color fg;
	SDL_Color bg;

	SDL_Color colour16[16];
} ColourScheme;

typedef struct Vec2 {
	int x;
	int y;
} Vec2;

typedef struct AttrInfo {
	uint8_t  fg;
	uint8_t  bg;
	uint16_t attr;
} AttrInfo;

typedef struct Cell {
	char     ch;
	AttrInfo attr;
} Cell;

Cell NewCell(char ch, uint8_t fg, uint8_t bg, uint16_t attr);
Cell CellByCharacter(char ch);

typedef struct TextScreen {
	Cell*         cells;
	Vec2          size;
	Vec2          cursor;
	ColourScheme* colours;
	AttrInfo      attr;
} TextScreen;

TextScreen TextScreen_New(uint32_t w, uint32_t h);
void       TextScreen_Free(TextScreen* text);
Cell       TextScreen_GetCharacter(TextScreen* text, int x, int y);
void       TextScreen_SetCharacter(TextScreen* text, int x, int y, Cell cell);
void       TextScreen_ScrollDown(TextScreen* text, int lines);
void       TextScreen_PutCharacter(TextScreen* text, char ch);
void       TextScreen_PutString(TextScreen* text, char* str);
void       TextScreen_Resize(TextScreen* text, Vec2 newSize);
void       TextScreen_Render(TextScreen* text, Video* video);

#endif
