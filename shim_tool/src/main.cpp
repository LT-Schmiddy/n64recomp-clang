#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>


#include "./globals.hpp"
#include "./config.hpp"

#define START_COMMAND_STR "--"

#ifdef _WIN32 
    #include <process.h>
    #include <windows.h>
    #include "subprocess.h"
#else
    #include <unistd.h>
#endif

bool requires_manual_command = false;
bool has_manual_command = false;
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
    std::string long_version_str = get_long_version_string();

    if (argc > 1 && argv[1][0] == '-') {
        requires_manual_command = true;
        for (; parse_size < argc; parse_size++){
            if (strncmp(argv[parse_size], START_COMMAND_STR, 3) == 0) {
                parse_size++;
                has_manual_command = true;
                break;
            }
        }
    }

    command_argc = argc - parse_size;
    command_argv = &argv[parse_size];
    
    cxxopts::Options options("nrs", long_version_str + "\nCommon tools for development of N64 Recompiled ports and mods.");
    options.add_options()
        ("h,help", "Prints usage infomation and then quits")
        ("l,list", "List available commands and then quits")
        ("i,info", "Print usage infomation and available commands then quits")
        ("d,debug", "Enable debug output")
        ("v,version", "Print version info and quits")
    ;

    options.custom_help("[OPTIONS... --] [SHIM COMMAND...]");

    try {
        global::option_args = options.parse(parse_size, argv);
    } catch (cxxopts::exceptions::exception e) {
        std::cerr << LOG_PREFIX "Error in parsing arguments - " << e.what() << std::endl;
        return 1;
    }

    // If we're only printing the help message, no need to load the config. So let's do this now.
    if (global::option_args.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (global::option_args.count("version")) {
        std::cout << get_version_string() << std::endl;
        return 0;
    }

    global::verbose = global::option_args[VERBOSE_KEY].as<bool>();

    // Initialize Globals
    // global::exec_dir = get_exec_path().parent_path();
    // global::config_file_path = fs::path(global::exec_dir).append(CONFIG_FILE_NAME);

    // Load Config:
    // if (!fs::exists(global::config_file_path)) {
    //     VCOUT << LOG_PREFIX "Missing config file at '" << global::config_file_path.string().c_str() << "'. Creating..." << std::endl;
    //     if (config_create_file()) {
    //         return 1;
    //     }
    // }
    // if (config_load_file()) {
    //     return 1;
    // }

    config_init();

    if (global::option_args.count("info")) {
        std::cout << options.help() << std::endl;
        config_list_commands();
        return 0;
    }


    if (global::option_args.count("list")) {
      config_list_commands();
      return 0;
    }

    // If there's no command, exit now:
    if (argc == 1 || (requires_manual_command && !has_manual_command) || global::option_args.count("info")) {
        std::cerr << LOG_PREFIX "No command specified. Printing usage information..." << std::endl;
        std::cout << options.help() << std::endl;
        config_list_commands();
        return 0;
    }

    VCOUT << LOG_PREFIX << "LOCATION = " << get_exec_path() << std::endl;


    // Finding command executable:
    std::string cmd_name = command_argv[0];
    VCOUT << "Command: " << cmd_name << std::endl;
    fs::path cmd_path(cmd_name);

    bool cmd_found = config_find_command(cmd_name, &cmd_path);

    if (!cmd_found) {
        std::cerr << LOG_PREFIX "Error - command '" << cmd_name << "' not found." << std::endl;
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
            std::cerr << LOG_PREFIX "Error - failed to run command '" << cmd_name << "' - Error code " << last_error << std::endl;
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