#include "sdl.h"
#include "util.h"
#include "constants.h"

Video Video_Init() {
	Video ret;

	// initialise SDL2
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		FATAL("Failed to initialise SDL2");
	}

	ret.window = SDL_CreateWindow(
		APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 720, 400,
		SDL_WINDOW_RESIZABLE
	);

	if (ret.window == NULL) {
		FATAL("Failed to create window");
	}

	ret.renderer = SDL_CreateRenderer(
		ret.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	);

	if (ret.renderer == NULL) {
		FATAL("Failed to create renderer");
	}

	// initialise SDL2_TTF
	if (TTF_Init() < 0) {
		FATAL("Failed to initialise SDL2_TTF");
	}

	ret.font = NULL;
	memset(ret.characters, 0, sizeof(ret.characters));

	return ret;
}

void Video_FreeFont(Video* video) {
	TTF_CloseFont(video->font);

	for (uint32_t i = 0; i < ARRAY_LEN(video->characters); ++ i) {
		if (video->characters[i] == NULL) {
			continue;
		}
		
		SDL_DestroyTexture(video->characters[i]);
		video->characters[i] = NULL;
	}
}

void Video_Free(Video* video) {
	Video_FreeFont(video);
	TTF_Quit();
	
	SDL_DestroyWindow(video->window);
	SDL_DestroyRenderer(video->renderer);
	SDL_Quit();
}

void Video_OpenFont(Video* video, char* path) {
	video->font = TTF_OpenFont(path, 16);

	if (video->font == NULL) {
		FATAL("Failed to open font"); // TODO: make a window for this and reset
	}

	int created = 0;

	// generate characters
	for (uint32_t i = 1; i < ARRAY_LEN(video->characters); ++ i) {
		if (i == 173) {
			continue; // idk
		}

		SDL_Color colour      = {255, 255, 255, 255};
		char      asString[2] = {(char) i, 0};

		SDL_Surface* textSurface = TTF_RenderText_Solid(
			video->font, asString, colour
		);

		if (textSurface == NULL) {
			fprintf(stderr, "TTF error for character %d: %s\n", i, TTF_GetError());
			//FATAL("Failed to render text");
			continue;
		}

		video->characters[i] = SDL_CreateTextureFromSurface(
			video->renderer, textSurface
		);

		if (video->characters[i] == NULL) {
			fprintf(stderr, "SDL error: %s\n", SDL_GetError());
			//FATAL("Failed to create texture");
			continue;
		}

		SDL_FreeSurface(textSurface);

		++ created;
	}

	printf("Rendered %d characters\n", created);

	const char* str = "M";
	TTF_SizeText(video->font, str, &video->charWidth, &video->charHeight);
}

void Video_DrawCharacter(Video* video, int x, int y, char ch, SDL_Colour colour) {
	SDL_Texture* texture = video->characters[(size_t) ch];

	if (texture == NULL) {
		texture = video->characters['?'];

		if (texture == NULL) {
			return;
		}
	}

	SDL_SetTextureColorMod(texture, colour.r, colour.g, colour.b);

	SDL_Rect textRect;
	textRect.x = x;
	textRect.y = y;

	SDL_QueryTexture(texture, NULL, NULL, &textRect.w, &textRect.h);

	SDL_RenderCopy(video->renderer, texture, NULL, &textRect);
}
