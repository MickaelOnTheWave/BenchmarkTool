set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)

add_subdirectory(libs/httplib)
add_subdirectory(libs/json)

add_executable(Backend src/main.cpp
   src/Server.h src/Server.cpp
   src/Database.h src/Database.cpp)

target_link_libraries(Backend PRIVATE
    httplib
    nlohmann_json
    Threads::Threads
    SQLite::SQLite3
)
target_include_directories(Backend PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../web)

set(ASSET_FILES ui/index.html ui/style.css ui/app.js schema/schema.sql)
set(RUNTIME_DIR ${CMAKE_BINARY_DIR}/Backend)
add_custom_target(AssetFiles
   COMMAND ${CMAKE_COMMAND} -E copy_directory
     ${CMAKE_CURRENT_SOURCE_DIR}/ui
     ${RUNTIME_DIR}/ui
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_CURRENT_SOURCE_DIR}/schema/schema.sql
      ${RUNTIME_DIR}/schema.sql
    SOURCES ${ASSET_FILES}
)

add_dependencies(Backend AssetFiles)
