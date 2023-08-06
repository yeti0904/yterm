#include "sdl.h"
#include "safe.h"
#include "util.h"
#include "input.h"
#include "terminal.h"
#include "constants.h"
#include "components.h"
#include "config.h"

const char* disclaimer[] = {
	"Copyright (c) 2023 yeti0904",
	"",
	"Permission is hereby granted, free of charge, to any person obtaining a copy",
	"of this software and associated documentation files (the \"Software\"), to deal",
	"in the Software without restriction, including without limitation the rights",
	"to use, copy, modify, merge, publish, distribute, sublicense, and/or sell",
	"copies of the Software, and to permit persons to whom the Software is",
	"furnished to do so, subject to the following conditions:",
	"",
	"The above copyright notice and this permission notice shall be included in all",
	"copies or substantial portions of the Software."
};

const char* usages[] = {
	"[OPTIONS]",
};

const char* execPath;

static void Usage(FILE* file) {
	args_usage_fprint(file, execPath, usages, ARRAY_LEN(usages), APP_DESC, true);
}

static void ParseFlags(args_t* args) {
	bool flagHelp    = false;
	bool flagVersion = false;
	flag_bool("h", "help",    "Show the usage",   &flagHelp);
	flag_bool("v", "version", "Show the version", &flagVersion);

	size_t where;
	bool   extra;
	if (args_parse_flags(args, &where, NULL, &extra) != 0) {
		FATAL("Error: '%s': %s\n", args->v[where], noch_get_err_msg());
	}
	else if (extra) {
		FATAL("Error: '%s': Unexpected argument\n", args->v[where]);
	}

	if (flagHelp) {
		Usage(stdout);
		exit(0);
	}

	if (flagVersion) {
		printf("%s %s by %s\n", APP_NAME, APP_VERSION, APP_AUTHOR);

		for (size_t i = 0; i < ARRAY_LEN(disclaimer); ++ i) {
			puts(disclaimer[i]);
		}

		exit(0);
	}
}

int main(int argc, const char** argv) {
	args_t args = args_new(argc, argv);
	execPath    = args_shift(&args);
	ParseFlags(&args);

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
	LoadConfig(&colourScheme);
	terminal.buffer.colours = &colourScheme;

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
							SetTerminalSize(&terminal);
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
