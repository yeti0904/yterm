#include "app.h"
#include "sdl.h"
#include "safe.h"
#include "input.h"
#include "config.h"
#include "constants.h"

void App_Init(App* app, char** env) {
	app->running = true;
	app->env     = env;

	// init config
	app->config.interpretEscapes = true;
	app->config.cursorEnabled    = true;

	// init video
	app->video = Video_Init();
	SDL_StartTextInput();
	Video_OpenFont(&app->video, "./font.ttf");

	// init tabs
	app->tabs       = NULL;
	app->tabAmount  = 0;
	app->currentTab = 0;
	App_AddTab(app);

	// init colourscheme
	LoadConfig(&app->colours);

	// init screen
	app->screen = TextScreen_New(
		app->video.windowSize.x / app->video.charWidth,
		app->video.windowSize.y / app->video.charHeight
	);
	app->screen.colours = &app->colours;
}

void App_Free(App* app) {
	// free screen
	TextScreen_Free(&app->screen);

	// free video
	Video_Free(&app->video);

	// free tabs
	free(app->tabs);
}

void App_AddTab(App* app) {
	if (app->tabs == NULL) {
		app->tabs = SafeMalloc(sizeof(Terminal));
	}
	else {
		app->tabs = SafeRealloc(
			app->tabs, (app->tabAmount + 1) * sizeof(Terminal)
		);
	}

	Terminal_Init(&app->tabs[app->tabAmount], app->env, App_GetUsableArea(app));

	app->tabs[app->tabAmount].buffer.colours = &app->colours;
	app->tabs[app->tabAmount].config         = &app->config;

	++ app->tabAmount;
}

Terminal* App_CurrentTab(App* app) {
	return &app->tabs[app->currentTab];
}

Vec2 App_GetUsableArea(App* app) {
	return (Vec2) {
		app->video.windowSize.x / app->video.charWidth,
		(app->video.windowSize.y / app->video.charHeight) - 1
	};
}

void App_UpdateTitle(App* app) {
	SDL_SetWindowTitle(app->video.window, App_CurrentTab(app)->title);
}

void App_Update(App* app) {
	SDL_Event e;
	
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
			case SDL_QUIT: {
				app->running = false;
				continue;
			}
			case SDL_KEYDOWN:
			case SDL_TEXTINPUT: {
				if (e.type == SDL_KEYDOWN) {
					const uint8_t* keys = SDL_GetKeyboardState(NULL);

					if (!ShiftPressed(keys) || !keys[SDL_SCANCODE_LCTRL]) {
						goto doInput;
					}

					if (
						(e.key.keysym.scancode == SDL_SCANCODE_LCTRL) ||
						(e.key.keysym.scancode == SDL_SCANCODE_LSHIFT) ||
						(e.key.keysym.scancode == SDL_SCANCODE_RSHIFT)
					) {
						goto doInput;
					}

					switch (e.key.keysym.scancode) {
						case SDL_SCANCODE_T: {
							App_AddTab(app);
							app->currentTab = app->tabAmount - 1;
							break;
						}
						case SDL_SCANCODE_PAGEUP: {
							if (app->currentTab == 0) {
								app->currentTab = app->tabAmount - 1;
							}
							else {
								-- app->currentTab;
							}
							break;
						}
						case SDL_SCANCODE_PAGEDOWN: {
							++ app->currentTab;

							if (app->currentTab >= app->tabAmount) {
								app->currentTab = 0;
							}
							break;
						}
						default: break;
					}

					break;
				}

				doInput:
				HandleInputEvent(&e, App_CurrentTab(app));
				break;
			}
			case SDL_WINDOWEVENT: {
				switch (e.window.event) {
					case SDL_WINDOWEVENT_RESIZED: {
						app->video.windowSize.x = e.window.data1;
						app->video.windowSize.y = e.window.data2;
					
						Vec2 newSize = {
							e.window.data1 / app->video.charWidth,
							e.window.data2 / app->video.charHeight
						};
						TextScreen_Resize(&app->screen, newSize);

						Vec2 newTabSize = App_GetUsableArea(app);
						
						for (size_t i = 0; i < app->tabAmount; ++ i) {
							TextScreen_Resize(&app->tabs[i].buffer, newTabSize);
						}
						
						SetTerminalSize(App_CurrentTab(app));
						break;
					}
				}
			}
		}
	}

	for (size_t i = 0; i < app->tabAmount; ++ i) {
		char* tabTitle = app->tabs[i].title;
		
		Terminal_Update(&app->tabs[i]);

		if (app->tabs[i].title != tabTitle) {
			App_UpdateTitle(app);
		}
	}

	app->screen.attr.attr |= ATTR_COLOUR_BG;
	app->screen.attr.bg    = COLOUR_GREY;
	app->screen.attr.fg    = COLOUR_BRIGHT_WHITE;
	TextScreen_HLine(&app->screen, (Vec2) {0, 0}, app->screen.size.x, ' ');

	char status[256];
	sprintf(
		status, "%s - [%d/%d]",
		APP_NAME, (int) app->currentTab + 1, (int) app->tabAmount
	);

	app->screen.cursor = (Vec2) {0, 0};
	TextScreen_PutString(&app->screen, status);

	app->screen.cursor = App_CurrentTab(app)->buffer.cursor;
	++ app->screen.cursor.y;
	TextScreen_Blit(&app->screen, &App_CurrentTab(app)->buffer, (Vec2) {0, 1});

	TextScreen_Render(&app->screen, &app->video);
	SDL_RenderPresent(app->video.renderer);
}
