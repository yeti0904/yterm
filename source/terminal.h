#ifndef YTERM_TERMINAL_H
#define YTERM_TERMINAL_H

#include "components.h"

struct PTY { // no clue what this could be
	int master, slave;
};

struct X11 {
	int      fd;
	Display* dpy;
	int      screen;
	Window   root;

	Window    termwin;
	GC        termgc;
	uint64_t  col_fg, col_bg;
	int32_t   w; // window size i think?
	int32_t   h;

	XFontStruct* xfont;
	int32_t      font_width;
	int32_t      font_height;

	char* buf;
	int   buf_w, buf_h;
	int   buf_x, buf_y;
	bool  readingEscape;
	char* escape;
};

bool SetTerminalSize(struct PTY* pty, struct X11* x11);
bool PtPair(struct PTY *pty);
bool Spawn(struct PTY *pty);

#endif
