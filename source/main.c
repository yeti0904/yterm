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

	// init colourscheme
	ColourScheme colourScheme;
	colourScheme.colour16[COLOUR_BLACK]          = HexToColour(0x212830);
	colourScheme.colour16[COLOUR_RED]            = HexToColour(0xC54133);
	colourScheme.colour16[COLOUR_GREEN]          = HexToColour(0x27AE60);
	colourScheme.colour16[COLOUR_YELLOW]         = HexToColour(0xEDB20A);
	colourScheme.colour16[COLOUR_BLUE]           = HexToColour(0x2479D0);
	colourScheme.colour16[COLOUR_MAGENTA]        = HexToColour(0x7D3EA0);
	colourScheme.colour16[COLOUR_CYAN]           = HexToColour(0x1D8579);
	colourScheme.colour16[COLOUR_WHITE]          = HexToColour(0xC9CCCD);
	colourScheme.colour16[COLOUR_GREY]           = HexToColour(0x2F3943);
	colourScheme.colour16[COLOUR_BRIGHT_RED]     = HexToColour(0xE74C3C);
	colourScheme.colour16[COLOUR_BRIGHT_GREEN]   = HexToColour(0x2ECC71);
	colourScheme.colour16[COLOUR_BRIGHT_YELLOW]  = HexToColour(0xF1C40F);
	colourScheme.colour16[COLOUR_BRIGHT_BLUE]    = HexToColour(0x3498DB);
	colourScheme.colour16[COLOUR_BRIGHT_MAGENTA] = HexToColour(0x9B59B6);
	colourScheme.colour16[COLOUR_BRIGHT_CYAN]    = HexToColour(0x2AA198);
	colourScheme.colour16[COLOUR_BRIGHT_WHITE]   = HexToColour(0xECF0F1);
	colourScheme.fg                              = HexToColour(0xC9CCCD);
	colourScheme.bg                              = HexToColour(0x161C24);
	terminal.buffer.colours                      = &colourScheme;

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
				case SDL_WINDOWEVENT: {
					switch (e.window.event) {
						case SDL_WINDOWEVENT_RESIZED: {
							Vec2 newSize = {
								e.window.data1 / terminal.video.charWidth,
								e.window.data2 / terminal.video.charHeight
							};

							TextScreen_Resize(&terminal.buffer, newSize);
							break;
						}
					}
				}
			}
		}
	
		Terminal_Update(&terminal);

		TextScreen_Render(&terminal.buffer, &terminal.video);
		SDL_RenderPresent(terminal.video.renderer);
	}

	TextScreen_Free(&terminal.buffer);
	Video_Free(&terminal.video);
}
