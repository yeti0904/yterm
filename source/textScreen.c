#include "safe.h"
#include "util.h"
#include "textScreen.h"

Cell NewCell(char ch, uint8_t fg, uint8_t bg, uint16_t attr) {
	Cell ret;
	ret.ch        = ch;
	ret.attr.fg   = fg;
	ret.attr.bg   = bg;
	ret.attr.attr = attr;
	return ret;
}

Cell CellByCharacter(char ch) {
	return NewCell(ch, 0, 0, ATTR_NONE);
}

AttrInfo NewAttr(uint8_t fg, uint8_t bg, uint16_t attr) {
	Cell cell = NewCell(0, fg, bg, attr);
	return cell.attr;
}

TextScreen TextScreen_New(uint32_t w, uint32_t h) {
	TextScreen ret;

	ret.cells  = SafeMalloc(w * h * sizeof(Cell));
	ret.size   = (Vec2) {.x = w, .y = h};
	ret.cursor = (Vec2) {.x = 0, .y = 0};

	for (size_t i = 0; i < w * h; ++ i) {
		ret.cells[i] = CellByCharacter(' ');
	}

	ret.attr = NewAttr(0, 0, ATTR_NONE);

	return ret;
}

void TextScreen_Free(TextScreen* text) {
	free(text->cells);
}

Cell TextScreen_GetCharacter(TextScreen* text, int x, int y) {
	return text->cells[(y * text->size.x) + x];
}

void TextScreen_SetCharacter(TextScreen* text, int x, int y, Cell cell) {
	if (
		(x < 0) ||
		(y < 0) ||
		(x >= text->size.x) ||
		(y >= text->size.y)
	) {
		return;
	}

	text->cells[(y * text->size.x) + x] = cell;
}

void TextScreen_ScrollDown(TextScreen* text, int lines) {
	for (int i = 0; i < lines; ++ i) {
		size_t remainingLength  = (text->size.x * text->size.y) - text->size.x;
		
		memmove(
			text->cells, text->cells + text->size.x,
			remainingLength * sizeof(Cell)
		);

		for (int i = 0; i < text->size.x; ++ i) {
			text->cells[remainingLength + i] = CellByCharacter(' ');
		}

		-- text->cursor.y;
	}
}

void TextScreen_PutCharacter(TextScreen* text, char ch) {
	switch (ch) {
		case '\n': {
			++ text->cursor.y;
			text->cursor.x = 0;

			if (text->cursor.y >= text->size.y) {
				TextScreen_ScrollDown(text, 1);
			}
			break;
		}
		case '\r': {
			text->cursor.x = 0;
			break;
		}
		case '\t': {
			text->cursor.x += 4;
			break;
		}
		case 0x07: { // bell
			puts("Ding!");
			break;
		}
		case 0x08: { // backspace
			-- text->cursor.x;

			if (text->cursor.x < 0) {
				text->cursor.x = 0;
			}
			
			TextScreen_SetCharacter(
				text, text->cursor.x, text->cursor.y, CellByCharacter(' ')
			);
			break;
		}
		default: {
			TextScreen_SetCharacter(
				text, text->cursor.x, text->cursor.y,
				NewCell(ch, text->attr.fg, text->attr.bg, text->attr.attr)
			);
			++ text->cursor.x;
			break;
		}
	}
	
	// wrap
	if (text->cursor.x >= text->size.x) {
		text->cursor.x = 0;
		++ text->cursor.y;
	}
}

void TextScreen_PutString(TextScreen* text, char* str) {
	for (size_t i = 0; i < strlen(str); ++ i) {
		TextScreen_PutCharacter(text, str[i]);
	}
}

void TextScreen_SetAttribute(TextScreen* text, uint16_t attr, bool on) {
	if (on) {
		text->attr.attr |= attr;
	}
	else {
		text->attr.attr &= ~attr;
	}
}

void TextScreen_Resize(TextScreen* text, Vec2 newSize) {
	size_t size     = newSize.x * newSize.y * sizeof(Cell);
	Cell*  newCells = (Cell*) SafeMalloc(size);

	for (size_t i = 0; i < size / sizeof(Cell); ++ i) {
		newCells[i] = CellByCharacter(' ');
	}

	for (int y = 0; y < text->size.y; ++ y) {
		if (y >= newSize.y) {
			break;
		}
		
		for (int x = 0; x < text->size.x; ++ x) {
			if (x >= newSize.x) {
				break;
			}
		
			Cell cell = TextScreen_GetCharacter(text, x, y);

			newCells[(y * newSize.x) + x] = cell;
		}
	}

	text->size = newSize;
	free(text->cells);
	text->cells = newCells;
}

void TextScreen_Render(TextScreen* text, Video* video) {
	SDL_SetRenderDrawColor(
		video->renderer,
		text->colours->bg.r, text->colours->bg.g, text->colours->bg.b, 255
	);
	SDL_RenderClear(video->renderer);

	for (int y = 0; y < text->size.y; ++ y) {
		for (int x = 0; x < text->size.x; ++ x) {
			SDL_Rect rect;
			rect.x = x * video->charWidth;
			rect.y = y * video->charHeight;
			rect.w = video->charWidth;
			rect.h = video->charHeight;

			Cell cell = TextScreen_GetCharacter(text, x, y);

			SDL_Color fg = text->colours->fg;
			SDL_Color bg = text->colours->bg;

			if (cell.attr.attr & ATTR_COLOUR_FG) {
				fg = text->colours->colour16[cell.attr.fg];
			}
			if (cell.attr.attr & ATTR_COLOUR_BG) {
				bg = text->colours->colour16[cell.attr.bg];
			}

			if ((x == text->cursor.x) && (y == text->cursor.y)) {
				SDL_Color temp = fg;
				fg             = bg;
				bg             = temp;
			}

			if (cell.attr.attr & ATTR_REVERSE) {
				SDL_Color temp = fg;
				fg             = bg;
				bg             = temp;
			}

			SDL_SetRenderDrawColor(video->renderer, bg.r, bg.g, bg.b, bg.a);
			SDL_RenderFillRect(video->renderer, &rect);

			Video_DrawCharacter(video, rect.x, rect.y, cell.ch, fg);
		}
	}
}
