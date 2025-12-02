#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>

// Windows


#include "subprocess.h"
#include "json.hpp"
#include "cxxopts.hpp"


#define START_COMMAND_STR "--"

#define LOG_PREFIX "N64R-SHIMS: "
#define CONFIG_FILE_NAME "n64r-shims-config.json"
#define SEARCH_PATHS_KEY "search-paths"
#define VERBOSE_KEY "verbose"
#define SHORTCUTS_KEY "shortcuts"

#ifdef _WIN32 
    #define EXEC_EXTENSION ".exe"
    #include <process.h>
    #include <windows.h>
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

#define CMD_ARG_TO_ARGC(arg) arg + parse_size
#define ARGC_TO_CMD_ARG(arg) arg - parse_size

bool has_command = false;
int parse_size = 1;
int command_argc = 0;
const char** command_argv = NULL;

fs::path get_exec_path() {
    #ifdef _WIN32
        char path_buf[MAX_PATH];
        DWORD result = GetModuleFileNameA(
            NULL,
            path_buf,
            MAX_PATH
        );
    #else
        char path_buf[PATH_MAX];
        ssize_t size;

        size = readlink("/proc/self/exe", path_buf, PATH_MAX);
    #endif
    return fs::path(path_buf);
}

int create_config_file() {
    ns::json out_config = {        
        {
            SEARCH_PATHS_KEY, {
                "./bin",
            }
        },
        {
            SHORTCUTS_KEY, {
                {"n", "./bin/N64Recomp" EXEC_EXTENSION},
                {"o", "./bin/OfflineModRecomp" EXEC_EXTENSION},
                {"m", "./bin/RecompModTool" EXEC_EXTENSION},
                {"r", "./bin/RSPRecomp" EXEC_EXTENSION},
            }
        },
        {VERBOSE_KEY, false},

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
    // Parsing Shortcuts
    ns::json shortcuts = global::config[SHORTCUTS_KEY];

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

#ifdef _WIN32
std::string windows_arg_string(const char** commandLine) {
    // Yes, I am blatently borrowing this code from subprocess.h
    char* commandLineCombined;
    subprocess_size_t len;
    int i, j;
    int need_quoting;

    // Combine commandLine together into a single string
    len = 0;
    for (i = 0; commandLine[i]; i++) {
        // for the trailing \0
        len++;

        // Quote the argument if it has a space in it
        if (strpbrk(commandLine[i], "\t\v ") != SUBPROCESS_NULL ||
                commandLine[i][0] == SUBPROCESS_NULL)
            len += 2;

        for (j = 0; '\0' != commandLine[i][j]; j++) {
            switch (commandLine[i][j]) {
            default:
                break;
            case '\\':
                if (commandLine[i][j + 1] == '"') {
                    len++;
                }

                break;
            case '"':
                len++;
                break;
            }
            len++;
        }
    }

    commandLineCombined = SUBPROCESS_CAST(char *, _alloca(len));
    if (!commandLineCombined) {
        return std::string("");
    }
    // Gonna re-use len to store the write index into commandLineCombined
    len = 0;
    for (i = 0; commandLine[i]; i++) {
        if (0 != i) {
            commandLineCombined[len++] = ' ';
        }
        need_quoting = strpbrk(commandLine[i], "\t\v ") != SUBPROCESS_NULL ||
                                     commandLine[i][0] == SUBPROCESS_NULL;
        if (need_quoting) {
            commandLineCombined[len++] = '"';
        }
        for (j = 0; '\0' != commandLine[i][j]; j++) {
            switch (commandLine[i][j]) {
            default:
                break;
            case '\\':
                if (commandLine[i][j + 1] == '"') {
                    commandLineCombined[len++] = '\\';
                }
                break;
            case '"':
                commandLineCombined[len++] = '\\';
                break;
            }
            commandLineCombined[len++] = commandLine[i][j];
        }
        if (need_quoting) {
            commandLineCombined[len++] = '"';
        }
    }
    commandLineCombined[len] = '\0';
    // And return;
    return std::string (commandLineCombined);
}
#endif

int main(int argc, const char** argv) {
    for (; parse_size < argc; parse_size++){
        if (strncmp(argv[parse_size], START_COMMAND_STR, 3) == 0) {
            parse_size++;
            has_command = true;
            command_argc = argc - parse_size;
            command_argv = &argv[parse_size];
            break;
        }
    }
    
    cxxopts::Options options("nrs", "N64 Recompiled Tool Collection Shim. Common tools for development of N64 Recompiled ports and mods.");
    options.add_options()
        ("h,help", "Print usage")
        ("v,verbose", "Enable verbose output");
    options.custom_help("[OPTIONS...] -- [SHIM COMMAND...]");

    auto result = options.parse(parse_size, argv);
    if (result.count("help")) {
      std::cout << options.help() << std::endl;
      return 0;
    }

    // Initialize Globals
    global::exec_dir = get_exec_path().parent_path();
    global::config_file_path = fs::path(global::exec_dir).append(CONFIG_FILE_NAME);

    // Load Config:
    if (!fs::exists(global::config_file_path)) {
        std::cout << LOG_PREFIX "Missing config file at '" << global::config_file_path.string().c_str() << "'. Creating..." << std::endl;
        if (create_config_file()) {
            return 1;
        }
    }
    if (load_config_file()) {
        return 1;
    }
    global::verbose = global::config[VERBOSE_KEY].get<bool>();
    VCOUT << LOG_PREFIX << "LOCATION = " << get_exec_path() << std::endl;

        // If there's no command, exit now:
    if (!has_command) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    // Finding command executable:
    std::string cmd_name = command_argv[0];
    VCOUT << "Command: " << cmd_name << std::endl;
    fs::path cmd_path(cmd_name);

    bool cmd_found = find_command(cmd_name, &cmd_path);

    if (!cmd_found) {
        std::cerr << LOG_PREFIX "Error: command '" << cmd_name << "' not found." << std::endl;
        return 1;
    }

    // Need the string object stored in memory:
    std::string cmd_str = cmd_path.string();

    // Creating new arg list;
    std::vector<const char*> cmd_list;
    cmd_list.push_back(cmd_str.c_str());
    for (int i = 1; i < command_argc; i++) {
        VCOUT << "Arg: " << command_argv[i] << std::endl;
        cmd_list.push_back(command_argv[i]);
    }
    cmd_list.push_back(NULL);

    // On Windows, we'll run the command as a subprocess.
    // On Posix systems, we'll use process image replacement instead.
    #ifdef _WIN32 
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);
        ZeroMemory( &pi, sizeof(pi) );

        std::string win_args = windows_arg_string(cmd_list.data());
        VCOUT << win_args <<std::endl;
        if( !CreateProcess( NULL,   // No module name (use command line)
            (char*)win_args.c_str(),        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &si,            // Pointer to STARTUPINFO structure
            &pi )           // Pointer to PROCESS_INFORMATION structure
        ) {
            DWORD last_error = GetLastError();
            std::cerr << LOG_PREFIX "Error: failed to run command '" << cmd_name << "' - Error code " << last_error << std::endl;
            return last_error;
        }
        DWORD exitCode;
        // Wait until child process exits.
        WaitForSingleObject( pi.hProcess, INFINITE );
        // Get the process return code.
        GetExitCodeProcess(pi.hProcess, &exitCode);
        // Close process and thread handles. 
        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );
        return exitCode;

    #else
        int retVal = execvp(cmd_str.c_str(), (char *const *)cmd_list.data());
        std::cerr << LOG_PREFIX "Error: failed to run command '" << cmd_name << "' - Error code " << retVal << std::endl;
        return retVal;
    #endif
}