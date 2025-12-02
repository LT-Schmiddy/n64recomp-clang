#include <fstream>
#include "./config.hpp"

int config_create_file() {
    ns::json out_config = {        
        {
            CONFIG_SEARCH_PATHS_KEY, {
                "./bin",
            }
        },
        {
            CONFIG_SHORTCUTS_KEY, {
                {"n", "./bin/N64Recomp" EXEC_EXTENSION},
                {"o", "./bin/OfflineModRecomp" EXEC_EXTENSION},
                {"m", "./bin/RecompModTool" EXEC_EXTENSION},
                {"r", "./bin/RSPRecomp" EXEC_EXTENSION},
            }
        }
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

int config_load_file() {
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
bool config_find_command(std::string cmd, fs::path* out_path) {
    // Parsing Shortcuts
    ns::json shortcuts = global::config[CONFIG_SHORTCUTS_KEY];

    if (shortcuts.contains(cmd)) {
        std::string cmd_path_str = shortcuts[cmd].get<std::string>();
        fs::path cmd_path(cmd_path_str);
        if (cmd_path.is_relative()) {
            cmd_path = fs::absolute(fs::path(global::exec_dir).append(cmd_path_str));
        }

        if (!fs::exists(cmd_path)) {
            std::cerr << LOG_PREFIX "Error: The binary for shortcut '" << cmd << "' ('" << cmd_path_str << "') does not exist." << std::endl;
            return false;
        }

        *out_path = cmd_path;
        return true;
        
    }

    // Searching for full command:
    ns::json search_dirs = global::config[CONFIG_SEARCH_PATHS_KEY];
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

void config_list_commands() {
    ns::json shortcuts = global::config[CONFIG_SHORTCUTS_KEY];

    std::cout << "Available Tools:" << std::endl;

    ns::json search_dirs = global::config[CONFIG_SEARCH_PATHS_KEY];
    for (ns::json::iterator it = search_dirs.begin(); it != search_dirs.end(); ++it) {
        std::string dir_str = it->get<std::string>();
        fs::path dir(dir_str);
        if (dir.is_relative()) {
            dir = fs::absolute(fs::path(global::exec_dir).append(dir_str));
        }

        std::cout << "Location: " << dir << std::endl;
        if (!fs::exists(dir)) {
            continue;
        }


        for (auto const& dir_entry : std::filesystem::directory_iterator(dir)) {
            fs::path candidate(dir_entry);

            if (candidate.filename().extension() == EXEC_EXTENSION) {
                std::cout << "\t" << candidate.filename().replace_extension("").string() << std::endl;
            }
        }
    }

    std::cout << "Shortcuts:" << std::endl;
    for (ns::json::iterator it = shortcuts.begin(); it != shortcuts.end(); ++it) {
        std::cout << "\t" << it.key().c_str() << " : " << it.value().get<std::string>() << std::endl;
    }
}