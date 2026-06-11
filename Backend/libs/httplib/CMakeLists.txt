# cpp-httplib (header-only library)
# Version: v0.14.3
# Simplified for use as a dependency

add_library(httplib INTERFACE)
target_include_directories(httplib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
