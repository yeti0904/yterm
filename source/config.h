#ifndef YTERM_CONFIG_H
#define YTERM_CONFIG_H

#include "components.h"
#include "util.h"
#include "textScreen.h"

#define CONFIG_FOLDER "~/.config/yterm/"
#define CONFIG_FILE   "config.json"
#define THEME_FOLDER  "themes/"

void LoadConfig(ColourScheme* colourScheme);

#endif
