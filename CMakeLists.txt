cmake_minimum_required(VERSION 3.15)
project(BenchmarkTool LANGUAGES CXX C)

# Common settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies (all in libs/ folder)
add_subdirectory(libs/httplib)
add_subdirectory(libs/json)

# CEF dependency
set(CEF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/libs/CEF" CACHE PATH "CEF SDK location")
list(APPEND CMAKE_MODULE_PATH "${CEF_ROOT}/cmake")
find_package(CEF REQUIRED)
# Build the libcef_dll_wrapper library
add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH})

# Project components
add_subdirectory(src/Backend)
add_subdirectory(src/Frontend)
