#include "config.h"

static const char* defaultConfig[] = {
	"{",
	"\t\"theme\": \"default\"",
	"}",
};

static const char* defaultTheme[] = {
	"{",
	"\t\"black\":          \"212830\",",
	"\t\"red\":            \"C54133\",",
	"\t\"green\":          \"27AE60\",",
	"\t\"yellow\":         \"EDB20A\",",
	"\t\"blue\":           \"2479D0\",",
	"\t\"magenta\":        \"7D3EA0\",",
	"\t\"cyan\":           \"1D8579\",",
	"\t\"white\":          \"C9CCCD\",",
	"\t\"grey\":           \"2F3943\",",
	"\t\"bright_red\":     \"E74C3C\",",
	"\t\"bright_green\":   \"2ECC71\",",
	"\t\"bright_yellow\":  \"F1C40F\",",
	"\t\"bright_blue\":    \"3498DB\",",
	"\t\"bright_magenta\": \"9B59B6\",",
	"\t\"bright_cyan\":    \"2AA198\",",
	"\t\"bright_white\":   \"ECF0F1\",",
	"\t\"fg\":             \"C9CCCD\",",
	"\t\"bg\":             \"161C24\"",
	"}",
};

static void WriteStringArrayToFile(const char **array, size_t size, const char *path) {
	FILE *file = fopen(path, "w");
	if (file == NULL) {
		FATAL("Could not write file \"%s\"", path);
	}

	for (size_t i = 0; i < size; ++ i) {
		fprintf(file, "%s\n", array[i]);
	}

	fclose(file);
}

static json_t* ParseJson(const char* path) {
	size_t  row, col;
	json_t* json = json_from_file(path, &row, &col);
	if (json == NULL) {
		if (noch_get_err() == NOCH_ERR_FOPEN) {
			FATAL("Could not open file \"%s\"", path);
		}
		else if (noch_get_err() == NOCH_ERR_PARSER) {
			FATAL("%s:%zu:%zu: %s", path, row, col, noch_get_err_msg());
		}
		else {
			FATAL("%s: %s", path, noch_get_err_msg());
		}
	}

	return json;
}

static const char* JsonGetString(json_t* json, const char* key, const char* path) {
	json_t* strJson = json_obj_at(json, key);
	if (strJson == NULL) {
		FATAL("%s: Key \"%s\" is missing", path, key);
	}
	else if (strJson->type != JSON_STR) {
		FATAL("%s: Key \"%s\" expected to be a string", path, key);
	}

	return strJson->as.str.buf;
}

static const char *colourToString[] = {
	[COLOUR_BRIGHT_WHITE]   = "bright_white",
	[COLOUR_BRIGHT_CYAN]    = "bright_cyan",
	[COLOUR_BRIGHT_MAGENTA] = "bright_magenta",
	[COLOUR_BRIGHT_BLUE]    = "bright_blue",
	[COLOUR_BRIGHT_YELLOW]  = "bright_yellow",
	[COLOUR_BRIGHT_GREEN]   = "bright_green",
	[COLOUR_BRIGHT_RED]     = "bright_red",
	[COLOUR_GREY]           = "grey",
	[COLOUR_WHITE]          = "white",
	[COLOUR_CYAN]           = "cyan",
	[COLOUR_MAGENTA]        = "magenta",
	[COLOUR_BLUE]           = "blue",
	[COLOUR_YELLOW]         = "yellow",
	[COLOUR_GREEN]          = "green",
	[COLOUR_RED]            = "red",
	[COLOUR_BLACK]          = "black",
};

void LoadConfig(ColourScheme* colourScheme) {
	assert(colourScheme != NULL);

	char path[2048];
	assert(CONFIG_FOLDER[0] == '~');

	const char* home = getenv("HOME");
	if (home == NULL) {
		FATAL("Failed to get the HOME environment variable");
	}

	strcpy(path, home);
    const char* config_path = CONFIG_FOLDER;
	strcat(path, config_path + 1); 

	if (access(path, F_OK) != 0) {
		if (mkdir(path, 0777) != 0) {
			FATAL("Failed to create config folder \"%s\"", path);
		}
	}

	char configPath[2048];
	strcpy(configPath, path);
	strcat(configPath, CONFIG_FILE);

	if (access(configPath, F_OK) != 0) {
		WriteStringArrayToFile(defaultConfig, ARRAY_LEN(defaultConfig), configPath);
	}

	char defaultThemePath[2048];
	strcpy(defaultThemePath, path);
	strcat(defaultThemePath, THEME_FOLDER);

	if (access(defaultThemePath, F_OK) != 0) {
		if (mkdir(defaultThemePath, 0777) != 0) {
			FATAL("Could not create directory \"%s\"", defaultThemePath);
		}
	}

	strcat(defaultThemePath, "default.json");

	if (access(defaultThemePath, F_OK) != 0) {
		WriteStringArrayToFile(
			defaultTheme, ARRAY_LEN(defaultTheme), defaultThemePath
		);
	}

	json_t*     json      = ParseJson(configPath);
	const char* themeName = JsonGetString(json, "theme", configPath);

	char themePath[2048];
	strcpy(themePath, path);
	strcat(themePath, THEME_FOLDER);
	strcat(themePath, themeName);
	strcat(themePath, ".json");

	json_t* themeJson = ParseJson(themePath);

	#define LOAD_COLOUR(KEY, PTR) \
		do { \
			if (!HexToColour(JsonGetString( \
				themeJson, KEY, themePath), PTR \
			)) { \
				FATAL("%s: Invalid hex provided for colour \"%s\"", themePath, KEY); \
			} \
		} while (0)

	for (size_t i = 0; i < ARRAY_LEN(colourToString); ++ i) {
		LOAD_COLOUR(colourToString[i], colourScheme->colour16 + i);
	}

	LOAD_COLOUR("fg", &colourScheme->fg);
	LOAD_COLOUR("bg", &colourScheme->bg);

#undef LOAD_COLOUR

	json_destroy(themeJson);
	json_destroy(json);
}
