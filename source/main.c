#include "sdl.h"
#include "safe.h"
#include "util.h"
#include "input.h"
#include "terminal.h"
#include "constants.h"
#include "components.h"

int main(void) {
	Terminal terminal;

	Terminal_Init(&terminal);

	// init video
	terminal.video = Video_Init();
	SDL_StartTextInput();
	Video_OpenFont(&terminal.video, "./font.ttf");
	terminal.buffer = TextScreen_New(80, 25);

	// init terminal
	PtPair(&terminal.pty);
	SetTerminalSize(&terminal);
	Spawn(&terminal.pty);

	while (terminal.running) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
				case SDL_QUIT: {
					terminal.running = false;
					continue;
				}
				case SDL_KEYDOWN:
				case SDL_TEXTINPUT: {
					HandleInputEvent(&e, &terminal);
					break;
				}
			}
		}
	
		Terminal_Update(&terminal);

		TextScreen_Render(&terminal.buffer, &terminal.video);
		SDL_RenderPresent(terminal.video.renderer);
	}
}
