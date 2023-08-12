#include "safe.h"
#include "util.h"
#include "escapes.h"

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

static uint8_t FGColour(int colour) {
	if (colour >= 30 && colour <= 37) {
		return colour - 30;
	}
	else if (colour >= 90 && colour <= 97) {
		return colour - 90 + COLOUR_GREY;
	}
	else {
		return (uint8_t) -1;
	}
}

static uint8_t BGColour(int colour) {
	if (colour < 40) {
		return (uint8_t) -1;
	}

	return FGColour(colour - 10);
}

#define NEXT_BYTE() if (!NextByte(terminal, &in)) {return;}

static void RunCommand(Terminal* terminal, char cmd, int* args, size_t argsCount) {
	// TODO: error checking
	switch (cmd) {
		// cursor commands
		case 'H':
		case 'f': { // move cursor home/move cursor to
			if (argsCount == 0) {
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
					default: {
						uint8_t colour;
						if ((colour = FGColour(args[i])) != (uint8_t) -1) {
							terminal->buffer.attr.attr |= ATTR_COLOUR_FG;
							terminal->buffer.attr.fg    = colour;
						}
						else if ((colour = BGColour(args[i])) != (uint8_t) -1) {
							terminal->buffer.attr.attr |= ATTR_COLOUR_BG;
							terminal->buffer.attr.bg    = colour;
						}
					}
				}
			}
			break;
		}
		default: {
			printf("Running command %c: ", cmd);

			for (size_t i = 0; i < argsCount; ++ i) {
				printf("%d ", args[i]);
			}
			putchar('\n');
		}
	}
}


void HandleEscape(Terminal* terminal) {
	char  in;
	char* readStr;

	NEXT_BYTE();

	if (in == '[') { // CSI
		NEXT_BYTE();

		if (isdigit(in)) { // command
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

			free(readStr);
		}
		else if (in == '?') {
			readStr    = SafeMalloc(1);
			readStr[0] = 0;
		
			NEXT_BYTE();
			while ((in != 'h') && (in != 'l')) {
				readStr = SafeRealloc(readStr, strlen(readStr) + 2);
				strncat(readStr, &in, 1);

				NEXT_BYTE();
			}

			int option = atoi(readStr);
			free(readStr);

			printf("Set option %d to %c\n", option, in);
		}
	}
	else if (in == ']') {
		NEXT_BYTE();
		if (in != '0') {
			return;
		}

		NEXT_BYTE();
		if (in != ';') {
			return;
		}

		readStr  = SafeMalloc(1);
		*readStr = 0;

		NEXT_BYTE();

		while (in != 7) {
			readStr = SafeRealloc(readStr, strlen(readStr) + 2);
			strncat(readStr, &in, 1);
			
			NEXT_BYTE();
		}

		free(terminal->title);
		terminal->title = readStr;

		printf("Title set to %s\n", readStr);
	}
}

#undef NEXT_BYTE
