#ifndef YTERM_TERMINAL_H
#define YTERM_TERMINAL_H

#include "components.h"
#include "sdl.h"
#include "textScreen.h"

typedef struct Pty { // no clue what this could be
	int master, slave;
} Pty;

typedef struct TerminalConfig {
	bool interpretEscapes;
	bool cursorEnabled;
} TerminalConfig;

typedef struct Terminal { // basically a tab
	Pty             pty;
	TextScreen      buffer;
	TerminalConfig* config;
	char*           title;
} Terminal;

// Terminal
void Terminal_Init(Terminal* terminal, char** env, Vec2 size);
void Terminal_Free(Terminal* terminal);
void Terminal_Update(Terminal* terminal);

// Pty
void SetTerminalSize(Terminal* terminal);
void PtPair(Pty* pty);
void Spawn(Pty* pty, char** env);

#endif
