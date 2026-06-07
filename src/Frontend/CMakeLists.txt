find_package(Threads REQUIRED)

add_executable(Frontend CefApp.cpp)

# Link against libcef_dll_wrapper and libcef.so
target_link_libraries(Frontend PRIVATE 
    libcef_dll_wrapper
    ${CEF_LIB_RELEASE}
    Threads::Threads
)

# CEF headers use relative includes like "include/cef_base.h"
# CEF_INCLUDE_PATH is set to ${CEF_ROOT} which contains the include/ dir
target_include_directories(Frontend PRIVATE ${CEF_INCLUDE_PATH})
