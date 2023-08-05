#include "safe.h"
#include "escapes.h"

enum EscapeMode {
	EscapeMode_Unknown,
	EscapeMode_Property,
	EscapeMode_Commands
};

static bool NextByte(Terminal* terminal, char* byte) {
	fd_set readable;
	char   in;

	FD_ZERO(&readable);
	FD_SET(terminal->pty.master, &readable);

	struct timeval selectTimeout;
	selectTimeout.tv_sec  = 1;
	selectTimeout.tv_usec = 0;

	if (
		select(
			terminal->pty.master + 1, &readable, NULL, NULL, &selectTimeout
		) == -1
	) {
		perror("select");
		exit(1);
	}

	if (!FD_ISSET(terminal->pty.master, &readable)) {
		return false;
	}

	if (read(terminal->pty.master, &in, 1) <= 0) {
		return false;
	}

	*byte = in;
	return true;
}

static void RunCommand(Terminal* terminal, char cmd, int* args, size_t argsCount) {
	// TODO: error checking
	switch (cmd) {
		// cursor commands
		case 'H':
		case 'f': { // move cursor home/move cursor to
			if (cmd == 'H' && argsCount == 0) {
				terminal->buffer.cursor = (Vec2) {0, 0};
				break;
			}

			terminal->buffer.cursor.x = args[1];
			terminal->buffer.cursor.y = args[0];
			break;
		}
		case 'A': { // move cursor up
			terminal->buffer.cursor.y -= args[0];

			if (terminal->buffer.cursor.y < 0) {
				terminal->buffer.cursor.y = 0;
			}
			break;
		}
		case 'B': { // move cursor down
			terminal->buffer.cursor.y += args[0];

			if (terminal->buffer.cursor.y >= terminal->buffer.size.y) {
				int difference =
					terminal->buffer.cursor.y - terminal->buffer.size.y;
					
				TextScreen_ScrollDown(&terminal->buffer, difference + 1);
			}
			break;
		}
		case 'C': { // move cursor right
			terminal->buffer.cursor.x += args[0];

			if (terminal->buffer.cursor.x >= terminal->buffer.size.x) {
				terminal->buffer.cursor.x = terminal->buffer.size.x - 1;
			}
			break;
		}
		case 'D': { // move cursor left
			terminal->buffer.cursor.x -= args[0];

			if (terminal->buffer.cursor.x < 0) {
				terminal->buffer.cursor.x = 0;
			}
			break;
		}
		// erase commands
		case 'J': {
			Cell emptyCell = CellByCharacter(' ');
			
			if ((argsCount == 0) || (args[0] == 0)) { // until end of line
				for (
					int i = terminal->buffer.cursor.x;
					i < terminal->buffer.size.x; ++ i
				) {
					TextScreen_SetCharacter(
						&terminal->buffer, i, terminal->buffer.cursor.y,
						emptyCell
					);
				}
			}
			else if (args[0] == 2) { // full screen
				size_t size = terminal->buffer.size.x * terminal->buffer.size.y;

				for (size_t i = 0; i < size; ++ i) {
					terminal->buffer.cells[i] = emptyCell;
				}

				terminal->buffer.cursor = (Vec2) {0, 0};
			}
			break;
		}
		// colours/graphics commands
		case 'm': {
			for (size_t i = 0; i < argsCount; ++ i) {
				switch (args[i]) {
					case 0: { // reset all modes
						terminal->buffer.attr = NewAttr(0, 0, ATTR_NONE);
						break;
					}
					case 1: { // set bold
						terminal->buffer.attr.attr |= ATTR_BOLD;
						break;
					}
					case 22: { // reset bold and dim
						terminal->buffer.attr.attr &= ~ATTR_BOLD;
						terminal->buffer.attr.attr &= ~ATTR_DIM;
						break;
					}
					case 2: { // set dim
						terminal->buffer.attr.attr |= ATTR_DIM;
						break;
					}
					case 3: { // set italic
						terminal->buffer.attr.attr |= ATTR_ITALIC;
						break;
					}
					case 23: { // reset italic
						terminal->buffer.attr.attr &= ~ATTR_ITALIC;
						break;
					}
					case 4: { // set underline
						terminal->buffer.attr.attr |= ATTR_UNDERLINE;
						break;
					}
					case 24: { // reset underline
						terminal->buffer.attr.attr &= ~ATTR_UNDERLINE;
						break;
					}
					case 5: { // set blinking
						terminal->buffer.attr.attr |= ATTR_BLINKING;
						break;
					}
					case 25: { // reset blinking
						terminal->buffer.attr.attr &= ~ATTR_BLINKING;
						break;
					}
					case 7: { // set reverse
						terminal->buffer.attr.attr |= ATTR_REVERSE;
						break;
					}
					case 27: { // reset reverse
						terminal->buffer.attr.attr &= ~ATTR_REVERSE;
						break;
					}
					case 8: { // set hidden
						terminal->buffer.attr.attr |= ATTR_HIDDEN;
						break;
					}
					case 28: { // reset hidden
						terminal->buffer.attr.attr &= ~ATTR_HIDDEN;
						break;
					}
					case 9: { // set strikethrough
						terminal->buffer.attr.attr |= ATTR_STRIKE;
						break;
					}
					case 29: { // reset strikethrough
						terminal->buffer.attr.attr &= ~ATTR_STRIKE;
						break;
					}
					case 39: { // reset fg
						terminal->buffer.attr.attr &= ~ATTR_COLOUR_FG;
						break;
					}
					case 49: { // reset bg
						terminal->buffer.attr.attr &= ~ATTR_COLOUR_BG;
						break;
					}
					// i am really very sorry for what i have done
					// i hope you can find it in your heart to forgive me
					case 30: { // fg black
						terminal->buffer.attr.attr |= ATTR_COLOUR_FG;
						terminal->buffer.attr.fg    = COLOUR_BLACK;
						break;
					}
					case 31: { // fg red
						terminal->buffer.attr.attr |= ATTR_COLOUR_FG;
						terminal->buffer.attr.fg    = COLOUR_RED;
						break;
					}
					case 32: { // fg green
						terminal->buffer.attr.attr |= ATTR_COLOUR_FG;
						terminal->buffer.attr.fg    = COLOUR_GREEN;
						break;
					}
					case 33: { // fg yellow
						terminal->buffer.attr.attr |= ATTR_COLOUR_FG;
						terminal->buffer.attr.fg    = COLOUR_YELLOW;
						break;
					}
					case 34: { // fg blue
						terminal->buffer.attr.attr |= ATTR_COLOUR_FG;
						terminal->buffer.attr.fg    = COLOUR_BLUE;
						break;
					}
					case 35: { // fg magenta
						terminal->buffer.attr.attr |= ATTR_COLOUR_FG;
						terminal->buffer.attr.fg    = COLOUR_MAGENTA;
						break;
					}
					case 36: { // fg cyan
						terminal->buffer.attr.attr |= ATTR_COLOUR_FG;
						terminal->buffer.attr.fg    = COLOUR_CYAN;
						break;
					}
					case 37: { // fg white
						terminal->buffer.attr.attr |= ATTR_COLOUR_FG;
						terminal->buffer.attr.fg    = COLOUR_WHITE;
						break;
					}
					case 90: { // fg grey
						terminal->buffer.attr.attr |= ATTR_COLOUR_FG;
						terminal->buffer.attr.fg    = COLOUR_GREY;
						break;
					}
					case 91: { // fg bright red
						terminal->buffer.attr.attr |= ATTR_COLOUR_FG;
						terminal->buffer.attr.fg    = COLOUR_BRIGHT_RED;
						break;
					}
					case 92: { // fg bright green
						terminal->buffer.attr.attr |= ATTR_COLOUR_FG;
						terminal->buffer.attr.fg    = COLOUR_BRIGHT_GREEN;
						break;
					}
					case 93: { // fg bright yellow
						terminal->buffer.attr.attr |= ATTR_COLOUR_FG;
						terminal->buffer.attr.fg    = COLOUR_BRIGHT_YELLOW;
						break;
					}
					case 94: { // fg bright blue
						terminal->buffer.attr.attr |= ATTR_COLOUR_FG;
						terminal->buffer.attr.fg    = COLOUR_BRIGHT_BLUE;
						break;
					}
					case 95: { // fg bright magenta
						terminal->buffer.attr.attr |= ATTR_COLOUR_FG;
						terminal->buffer.attr.fg    = COLOUR_BRIGHT_MAGENTA;
						break;
					}
					case 96: { // fg bright cyan
						terminal->buffer.attr.attr |= ATTR_COLOUR_FG;
						terminal->buffer.attr.fg    = COLOUR_BRIGHT_CYAN;
						break;
					}
					case 97: { // fg white
						terminal->buffer.attr.attr |= ATTR_COLOUR_FG;
						terminal->buffer.attr.fg    = COLOUR_BRIGHT_WHITE;
						break;
					}
					case 40: { // bg black
						terminal->buffer.attr.attr |= ATTR_COLOUR_BG;
						terminal->buffer.attr.fg    = COLOUR_BLACK;
						break;
					}
					case 41: { // bg red
						terminal->buffer.attr.attr |= ATTR_COLOUR_BG;
						terminal->buffer.attr.fg    = COLOUR_RED;
						break;
					}
					case 42: { // bg green
						terminal->buffer.attr.attr |= ATTR_COLOUR_BG;
						terminal->buffer.attr.fg    = COLOUR_GREEN;
						break;
					}
					case 43: { // bg yellow
						terminal->buffer.attr.attr |= ATTR_COLOUR_BG;
						terminal->buffer.attr.fg    = COLOUR_YELLOW;
						break;
					}
					case 44: { // bg blue
						terminal->buffer.attr.attr |= ATTR_COLOUR_BG;
						terminal->buffer.attr.fg    = COLOUR_BLUE;
						break;
					}
					case 45: { // bg magenta
						terminal->buffer.attr.attr |= ATTR_COLOUR_BG;
						terminal->buffer.attr.fg    = COLOUR_MAGENTA;
						break;
					}
					case 46: { // bg cyan
						terminal->buffer.attr.attr |= ATTR_COLOUR_BG;
						terminal->buffer.attr.fg    = COLOUR_CYAN;
						break;
					}
					case 47: { // bg white
						terminal->buffer.attr.attr |= ATTR_COLOUR_BG;
						terminal->buffer.attr.fg    = COLOUR_WHITE;
						break;
					}
					case 100: { // bg bright black
						terminal->buffer.attr.attr |= ATTR_COLOUR_BG;
						terminal->buffer.attr.fg    = COLOUR_GREY;
						break;
					}
					case 101: { // bg bright  red
						terminal->buffer.attr.attr |= ATTR_COLOUR_BG;
						terminal->buffer.attr.fg    = COLOUR_BRIGHT_RED;
						break;
					}
					case 102: { // bg bright green
						terminal->buffer.attr.attr |= ATTR_COLOUR_BG;
						terminal->buffer.attr.fg    = COLOUR_BRIGHT_GREEN;
						break;
					}
					case 103: { // bg bright yellow
						terminal->buffer.attr.attr |= ATTR_COLOUR_BG;
						terminal->buffer.attr.fg    = COLOUR_BRIGHT_YELLOW;
						break;
					}
					case 104: { // bg bright blue
						terminal->buffer.attr.attr |= ATTR_COLOUR_BG;
						terminal->buffer.attr.fg    = COLOUR_BRIGHT_BLUE;
						break;
					}
					case 105: { // bg bright magenta
						terminal->buffer.attr.attr |= ATTR_COLOUR_BG;
						terminal->buffer.attr.fg    = COLOUR_BRIGHT_MAGENTA;
						break;
					}
					case 106: { // bg bright cyan
						terminal->buffer.attr.attr |= ATTR_COLOUR_BG;
						terminal->buffer.attr.fg    = COLOUR_BRIGHT_CYAN;
						break;
					}
					case 107: { // bg bright white
						terminal->buffer.attr.attr |= ATTR_COLOUR_BG;
						terminal->buffer.attr.fg    = COLOUR_BRIGHT_WHITE;
						break;
					}
				}
			}
		}
	}
}

#define NEXT_BYTE() if (!NextByte(terminal, &in)) {return;}

void HandleEscape(Terminal* terminal) {
	char  in;
	char* readStr;

	NEXT_BYTE();

	if (in != '[') {
		return;
	}

	NEXT_BYTE();

	if (isdigit(in)) {
		// TODO: maybe i should make this not a static array
		int    args[256];
		size_t argsCount = 0;
		
		readStr    = SafeMalloc(1);
		readStr[0] = 0;

		bool readingArgs = true;

		while (readingArgs) {
			readStr = SafeRealloc(readStr, strlen(readStr) + 2);
			strncat(readStr, &in, 1);
			NEXT_BYTE();

			if (!isdigit(in)) {
				args[argsCount] = atoi(readStr);
				++ argsCount;
				free(readStr);
				readStr    = SafeMalloc(1);
				readStr[0] = 0;

				if (in != ';') {
					readingArgs = false;
					RunCommand(terminal, in, args, argsCount);
				}
			}
		}
	}
}

#undef NEXT_BYTE
