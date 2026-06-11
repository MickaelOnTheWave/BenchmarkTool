cmake_minimum_required(VERSION 3.15)

project(BenchmarkTool LANGUAGES CXX C)

add_subdirectory(Backend)
#add_subdirectory(Frontend)


set(PROJECT_FILES TODOs.txt)
add_custom_target(ProjectFiles SOURCES ${PROJECT_FILES})
