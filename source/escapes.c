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
