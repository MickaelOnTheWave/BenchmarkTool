# nlohmann/json (header-only library)
# Version: v3.11.2
# Simplified for use as a dependency

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
