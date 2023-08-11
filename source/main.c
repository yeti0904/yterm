#include "app.h"
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

static void ParseFlags(args_t* args, TerminalConfig* config) {
	bool flagHelp     = false;
	bool flagVersion  = false;
	bool flagNoEscape = false;
	
	flag_bool("h", "help",    "Show the usage",   &flagHelp);
	flag_bool("v", "version", "Show the version", &flagVersion);
	flag_bool(
		"e", "no-escape", "Disables interpreting escape sequences", &flagNoEscape
	);

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

	if (flagNoEscape) {
		config->interpretEscapes = false;
	}
}

int main(int argc, const char** argv, char** env) {
	App app;
	App_Init(&app, env);

	args_t args = args_new(argc, argv);
	execPath    = args_shift(&args);
	ParseFlags(&args, &app.config);

	while (app.running) {
		App_Update(&app);
	}

	App_Free(&app);
}
