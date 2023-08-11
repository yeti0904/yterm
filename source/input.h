#ifndef YTERM_INPUT_H
#define YTERM_INPUT_H

#include "terminal.h"
#include "components.h"

bool ShiftPressed(const uint8_t* keys);
void HandleInputEvent(SDL_Event* e, Terminal* terminal);

#endif
