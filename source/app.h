#ifndef YTERM_APP_H
#define YTERM_APP_H

#include "components.h"
#include "textScreen.h"
#include "terminal.h"

typedef struct App {
	bool           running;
	Video          video;
	Terminal*      tabs;
	size_t         tabAmount;
	size_t         currentTab;
	ColourScheme   colours;
	TerminalConfig config;
	TextScreen     screen;
	char**         env;
} App;

void      App_Init(App* app, char** env);
void      App_Free(App* app);
void      App_AddTab(App* app);
Terminal* App_CurrentTab(App* app);
Vec2      App_GetUsableArea(App* app);
void      App_Update(App* app);

#endif
