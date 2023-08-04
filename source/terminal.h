#ifndef YTERM_TERMINAL_H
#define YTERM_TERMINAL_H

#include "components.h"
#include "sdl.h"
#include "textScreen.h"

typedef struct Pty { // no clue what this could be
	int master, slave;
} Pty;

typedef struct Terminal {
	Pty        pty;
	bool       running;
	Video      video;
	TextScreen buffer;
	bool       inEscape;
} Terminal;

// Terminal
void Terminal_Init(Terminal* terminal);
void Terminal_Update(Terminal* terminal);

// Pty
void SetTerminalSize(Terminal* terminal);
void PtPair(Pty* pty);
void Spawn(Pty* pty);

#endif
