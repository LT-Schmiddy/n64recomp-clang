#pragma once
#include <string>
#include <string.h>

#define LONG_PROGRAM_NAME "N64Recompiled Tool Collection Shim"

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 0

#define CONFIG_REVISION 0

inline std::string get_version_string() {
    char version_buf[10];
    snprintf(version_buf, sizeof(version_buf), "%u.%u.%u", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    return std::string(version_buf);
}

inline std::string get_long_version_string() {
    return LONG_PROGRAM_NAME " v" + get_version_string();
}