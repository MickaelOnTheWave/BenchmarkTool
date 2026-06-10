set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)

add_subdirectory(libs/httplib)
add_subdirectory(libs/json)

add_executable(Backend src/Server.cpp
   src/Database.h src/Database.cpp)

target_link_libraries(Backend PRIVATE
    httplib
    nlohmann_json
    Threads::Threads
    SQLite::SQLite3
)
target_include_directories(Backend PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../web)

set(PROJECT_FILES schema/schema.sql)
add_custom_target(ProjectFiles SOURCES ${PROJECT_FILES})

add_custom_command(TARGET Backend POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
   ${CMAKE_CURRENT_SOURCE_DIR}/schema/schema.sql
   $<TARGET_FILE_DIR:Backend>/schema.sql
)
