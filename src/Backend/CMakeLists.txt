find_package(Threads REQUIRED)

add_executable(Backend Server.cpp)
target_link_libraries(Backend PRIVATE 
    httplib
    nlohmann_json
    Threads::Threads
)
target_include_directories(Backend PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../web)
