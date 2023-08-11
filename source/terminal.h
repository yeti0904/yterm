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
} TerminalConfig;

typedef struct Terminal {
	Pty             pty;
	TextScreen      buffer;
	TerminalConfig* config;
} Terminal;

// Terminal
void Terminal_Init(Terminal* terminal, char** env, Vec2 size);
void Terminal_Update(Terminal* terminal);

// Pty
void SetTerminalSize(Terminal* terminal);
void PtPair(Pty* pty);
void Spawn(Pty* pty, char** env);

#endif
