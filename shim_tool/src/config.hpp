#pragma once

#include "globals.hpp"

#define CONFIG_FILE_NAME "n64r-shims-config.json"
#define CONFIG_SEARCH_PATHS_KEY "search-paths"
#define CONFIG_SHORTCUTS_KEY "shortcuts"
#define CONFIG_SHORTCUT_PATH_KEY "path"
#define CONFIG_SHORTCUT_COMMENT_KEY "comment"

int config_create_file();
int config_load_file();
bool config_find_command(std::string cmd, fs::path* out_path);
void config_list_commands();

