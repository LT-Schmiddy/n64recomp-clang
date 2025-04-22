#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>

// Windows


#include "subprocess.h"
#include "json.hpp"

#define LOG_PREFIX "N64R-SHIMS: "
#define CONFIG_FILE_NAME "n64r-shims-config.json"
#define SEARCH_PATHS_KEY "search-paths"
#define VERBOSE_KEY "verbose"

#ifdef _WIN32 
    #define EXEC_EXTENSION ".exe"
    #include <process.h>
    #define execvp _execvp
#else
    #include <unistd.h>
    #define EXEC_EXTENSION ""
#endif

namespace fs = std::filesystem;
namespace ns = nlohmann;
namespace global {
    fs::path exec_dir;
    fs::path config_file_path;
    ns::json config;
    bool verbose = true;
}
#define VCOUT if (global::verbose) std::cout

// Config
int create_config_file() {
    ns::json out_config = {        
        {
            SEARCH_PATHS_KEY, {
                "./bin",
                "./bin_essentials"
            }
        },
        {VERBOSE_KEY, false}
    };

    std::ofstream out_file(global::config_file_path);
    if (out_file.is_open()) {
        out_file << out_config.dump(4);
        out_file.close();
    } else {
        std::cerr << LOG_PREFIX "Error: Config at '" << global::config_file_path.string().c_str() << "' could not be created." << std::endl;
        return 1;
    }

    return 0;
}

int load_config_file() {
    std::ifstream in_file(global::config_file_path);
    if (in_file.is_open()) {
        try {
            in_file >> global::config;
        }
        catch (ns::json::parse_error& ex) {
            std::cerr << LOG_PREFIX "JSON parse error at byte " << ex.byte << std::endl;
            return 1;
        }
        in_file.close();
    } else {
        std::cerr << LOG_PREFIX "Error: Config at '" << global::config_file_path.string().c_str() << "' could not be opened." << std::endl;
        return 1;
    }
    return 0;
}

// Command Processing:
bool find_command(std::string cmd, fs::path* out_path) {
    ns::json search_dirs = global::config[SEARCH_PATHS_KEY];

    for (ns::json::iterator it = search_dirs.begin(); it != search_dirs.end(); ++it) {
        std::string dir_str = it->get<std::string>();
        fs::path dir(dir_str);
        if (dir.is_relative()) {
            dir = fs::absolute(fs::path(global::exec_dir).append(dir_str));
        }
        VCOUT << "Searching " << dir << std::endl;

        if (!fs::exists(dir)) {
            continue;
        }

        for (auto const& dir_entry : std::filesystem::directory_iterator(dir)) {
            VCOUT << dir_entry << '\n';

            fs::path candidate(dir_entry);
            if (candidate.filename().replace_extension("").string() == cmd) {
                VCOUT << "FOUND " << candidate << '\n';
                *out_path = candidate;
                return true;
            }
        }
    }
    return false;
}

int main(int argc, char** argv) {
    // If there's no command, exit now:
    if (argc < 2) {
        std::cerr << LOG_PREFIX "Error: No command specified." << std::endl;
        return 1;
    }

    // Initialize Globals
    global::exec_dir = fs::path(argv[0]).parent_path();
    global::config_file_path = fs::path(global::exec_dir).append(CONFIG_FILE_NAME);

    // Load Config:
    if (!fs::exists(global::config_file_path)) {
        VCOUT << LOG_PREFIX "Missing config file at '" << global::config_file_path.string().c_str() << "'. Creating..." << std::endl;
        if (create_config_file()) {
            return 1;
        }
    }
    if (load_config_file()) {
        return 1;
    }
    global::verbose = global::config[VERBOSE_KEY].get<bool>();
    
    // Finding command executable:
    std::string cmd_name = argv[1];
    VCOUT << "Command: " << cmd_name << std::endl;
    fs::path cmd_path(cmd_name);
    find_command(cmd_name, &cmd_path);

    // Need the string object stored in memory:
    std::string cmd_str = cmd_path.string();

    // Creating new arg list;
    std::vector<const char*> cmd_list;
    cmd_list.push_back(cmd_str.c_str());
    for (int i = 2; i < argc; i++) {
        VCOUT << "Arg: " << argv[i] << std::endl;
        cmd_list.push_back(argv[i]);
    }
    cmd_list.push_back(NULL);

    // Run:
    int retVal = execvp(cmd_str.c_str(), cmd_list.data());
    std::cerr << LOG_PREFIX "Error: failed to run command '" << cmd_name << "' - Error code " << retVal << std::endl;
    return retVal;
}