#include "util.h"
#include "input.h"

bool ShiftPressed(const uint8_t* keys) {
	return (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT])? true : false;
}

static char* KeyToSequence(SDL_Scancode key) {
	const uint8_t* keys = SDL_GetKeyboardState(NULL);

	if (keys[SDL_SCANCODE_LCTRL] && ShiftPressed(keys)) {
		switch (key) {
			case SDL_SCANCODE_BACKSPACE: return DupString("\x08");
			case SDL_SCANCODE_RETURN:    return DupString("\n");
			case SDL_SCANCODE_F1:        return DupString("\x1b[1;6P");
			case SDL_SCANCODE_F2:        return DupString("\x1b[1;6Q");
			case SDL_SCANCODE_F3:        return DupString("\x1bOR");
			case SDL_SCANCODE_F4:        return DupString("\x1b[1;6S");
			case SDL_SCANCODE_F5:        return DupString("\x1b[15;6~");
			case SDL_SCANCODE_F6:        return DupString("\x1b[17;6~");
			case SDL_SCANCODE_F7:        return DupString("\x1b[18;6~");
			case SDL_SCANCODE_F8:        return DupString("\x1b[19;6~");
			case SDL_SCANCODE_F9:        return DupString("\x1b[20;6~");
			case SDL_SCANCODE_F10:       return DupString("\x1b[21;6~");
			case SDL_SCANCODE_F11:       return DupString("\x1b[23;6~");
			case SDL_SCANCODE_F12:       return DupString("\x1b[24;6~");
			default:                     return NULL;
		}
	}
	else if (keys[SDL_SCANCODE_LCTRL]) {
		switch (key) {
			case SDL_SCANCODE_BACKSPACE: return DupString("\x7F");
			case SDL_SCANCODE_RETURN:    return DupString("\n");
			case SDL_SCANCODE_F1:        return DupString(""); // lxterminal does nothing for these?
			case SDL_SCANCODE_F2:        return DupString("");
			case SDL_SCANCODE_F3:        return DupString("");
			case SDL_SCANCODE_F4:        return DupString("");
			case SDL_SCANCODE_F5:        return DupString("\x1b[15;5~");
			case SDL_SCANCODE_F6:        return DupString("\x1b[17;5~");
			case SDL_SCANCODE_F7:        return DupString("\x1b[18;5~");
			case SDL_SCANCODE_F8:        return DupString("\x1b[19;5~");
			case SDL_SCANCODE_F9:        return DupString("\x1b[20;5~");
			case SDL_SCANCODE_F10:       return DupString("\x1b[21;5~");
			case SDL_SCANCODE_F11:       return DupString("\x1b[23;5~");
			case SDL_SCANCODE_F12:       return DupString("\x1b[24;5~");
			case SDL_SCANCODE_A:         return DupString("\x01");
			case SDL_SCANCODE_B:         return DupString("\x02");
			case SDL_SCANCODE_C:         return DupString("\x03"); // might be wrong
			case SDL_SCANCODE_D:         return DupString("\x04");
			case SDL_SCANCODE_E:         return DupString("\x05");
			case SDL_SCANCODE_F:         return DupString("\x06");
			case SDL_SCANCODE_G:         return DupString("\x07");
			case SDL_SCANCODE_H:         return DupString("\x08");
			case SDL_SCANCODE_I:         return DupString("\x09");
			case SDL_SCANCODE_J:         return DupString("\x0A");
			case SDL_SCANCODE_K:         return DupString("\x0B");
			case SDL_SCANCODE_L:         return DupString("\x0C");
			case SDL_SCANCODE_M:         return DupString("\x0A");
			case SDL_SCANCODE_N:         return DupString("\x0E");
			case SDL_SCANCODE_O:         return DupString("\x0F");
			case SDL_SCANCODE_P:         return DupString("\x10");
			case SDL_SCANCODE_Q:         return DupString("");
			case SDL_SCANCODE_R:         return DupString("\x12");
			case SDL_SCANCODE_S:         return DupString("");
			case SDL_SCANCODE_T:         return DupString("");
			case SDL_SCANCODE_U:         return DupString("");
			case SDL_SCANCODE_V:         return DupString("");
			case SDL_SCANCODE_W:         return DupString("");
			case SDL_SCANCODE_X:         return DupString("");
			case SDL_SCANCODE_Y:         return DupString("");
			case SDL_SCANCODE_Z:         return DupString("");
			default:                     return NULL;
		}
	}
	else if (ShiftPressed(keys)) {
		switch (key) {
			case SDL_SCANCODE_BACKSPACE: return DupString("\x7F");
			case SDL_SCANCODE_RETURN:    return DupString("\n");
			case SDL_SCANCODE_F1:        return DupString("\x1b[1;2P");
			case SDL_SCANCODE_F2:        return DupString("\x1b[1;2Q");
			case SDL_SCANCODE_F3:        return DupString("\x1b[1;2R");
			case SDL_SCANCODE_F4:        return DupString("\x1b[1;2S");
			case SDL_SCANCODE_F5:        return DupString("\x1b[15;2~");
			case SDL_SCANCODE_F6:        return DupString("\x1b[17;2~");
			case SDL_SCANCODE_F7:        return DupString("\x1b[18;2~");
			case SDL_SCANCODE_F8:        return DupString("\x1b[19;2~");
			case SDL_SCANCODE_F9:        return DupString("\x1b[20;2~");
			case SDL_SCANCODE_F10:       return DupString("\x1b[21;2~");
			case SDL_SCANCODE_F11:       return DupString("\x1b[23;2~");
			case SDL_SCANCODE_F12:       return DupString("\x1b[24;2~");
			default:                     return NULL;
		}
	}
	else {
		switch (key) {
			case SDL_SCANCODE_BACKSPACE: return DupString("\x08");
			case SDL_SCANCODE_RETURN:    return DupString("\n");
			case SDL_SCANCODE_F1:        return DupString("\x1bOP");
			case SDL_SCANCODE_F2:        return DupString("\x1bOQ");
			case SDL_SCANCODE_F3:        return DupString("\x1bOR");
			case SDL_SCANCODE_F4:        return DupString("\x1bOS");
			case SDL_SCANCODE_F5:        return DupString("\x1b[15~");
			case SDL_SCANCODE_F6:        return DupString("\x1b[17~");
			case SDL_SCANCODE_F7:        return DupString("\x1b[18~");
			case SDL_SCANCODE_F8:        return DupString("\x1b[19~");
			case SDL_SCANCODE_F9:        return DupString("\x1b[20~");
			case SDL_SCANCODE_F10:       return DupString("\x1b[21~");
			case SDL_SCANCODE_F11:       return DupString("\x1b[23~");
			case SDL_SCANCODE_F12:       return DupString("\x1b[24~");
			default:                     return NULL;
		}
	}
}

void HandleInputEvent(SDL_Event* e, Terminal* terminal) {
	switch (e->type) {
		case SDL_TEXTINPUT: {
			write(terminal->pty.master, e->text.text, strlen(e->text.text));
			break;
		}
		case SDL_KEYDOWN: {
			char* sequence = KeyToSequence(e->key.keysym.scancode);

			if (sequence == NULL) {
				break;
			}

			write(terminal->pty.master, sequence, strlen(sequence));
			free(sequence);
			break;
		}
	}
}
