#pragma once

#include <iostream>
#include <filesystem>
#include "json.hpp"
#include "cxxopts.hpp"

namespace fs = std::filesystem;
namespace ns = nlohmann;
namespace global {
    inline fs::path exec_dir;
    inline fs::path config_file_path;
    inline ns::json config;
    inline cxxopts::ParseResult option_args;
    inline bool verbose = false;
    
}
#define VCOUT if (global::verbose) std::cout

#ifdef _WIN32 
    #define EXEC_EXTENSION ".exe"
#else
    #define EXEC_EXTENSION ""
#endif

#define LOG_PREFIX "N64R-SHIMS: "
#define CONFIG_FILE_NAME "n64r-shims-config.json"
#define SEARCH_PATHS_KEY "search-paths"
#define VERBOSE_KEY "verbose"
#define SHORTCUTS_KEY "shortcuts"