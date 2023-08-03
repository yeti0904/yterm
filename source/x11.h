#ifndef YTERM_X11_H
#define YTERM_X11_H

// Xlib is bloat
// one day this might use SDL2 or maybe even that new SDL3 thing

#include "components.h"
#include "terminal.h"

void X11_Key(XKeyEvent* ev, struct PTY* pty);
void X11_Redraw(struct X11* x11);
bool X11_Setup(struct X11* x11);

#endif
