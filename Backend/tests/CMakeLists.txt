find_package(Catch2 3 REQUIRED)

file(GLOB TEST_SOURCES
    src/*.cpp
)

add_executable(UnitTests
    ${TEST_SOURCES}
)

target_link_libraries(UnitTests PRIVATE
    Core
    Catch2::Catch2WithMain
    nlohmann_json
)

target_include_directories(UnitTests PRIVATE
    ${CMAKE_SOURCE_DIR}/Backend/src
 )

include(CTest)
include(Catch)

catch_discover_tests(UnitTests)
