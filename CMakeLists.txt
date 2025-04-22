cmake_minimum_required(VERSION 3.20)

project(n64recomp-clang-shims)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SHIM_TOOL_NAME n64r)

add_executable(${SHIM_TOOL_NAME})

add_subdirectory("./src")
include_directories("./includes")

set_target_properties(${SHIM_TOOL_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "./arc/"
    LIBRARY_OUTPUT_DIRECTORY "./lib/"
    RUNTIME_OUTPUT_DIRECTORY "./bin/"
)
