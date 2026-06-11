
set(UiFiles index.html)
add_custom_target(UiFiles SOURCES ${UiFiles})

# find_package(Threads REQUIRED)

# # CEF dependency
# set(CEF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/libs/CEF" CACHE PATH "CEF SDK location")
# list(APPEND CMAKE_MODULE_PATH "${CEF_ROOT}/cmake")
# # Build the libcef_dll_wrapper library
# add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH})
# find_package(CEF REQUIRED)

# add_executable(Frontend CefApp.cpp)

# # Ensure the executable can find libcef.so in its own directory
# set_target_properties(Frontend PROPERTIES
#     BUILD_RPATH "$ORIGIN"
#     INSTALL_RPATH "$ORIGIN"
# )

# # Link against libcef_dll_wrapper and libcef.so
# target_link_libraries(Frontend PRIVATE
#     libcef_dll_wrapper
#     ${CEF_LIB_RELEASE}
#     Threads::Threads
# )

# # CEF headers use relative includes like "include/cef_base.h"
# # CEF_INCLUDE_PATH is set to ${CEF_ROOT} which contains the include/ dir
# target_include_directories(Frontend PRIVATE ${CEF_INCLUDE_PATH})

# # Copy required CEF files next to the executable for runtime
# add_custom_command(TARGET Frontend POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy
#         "${CEF_ROOT}/Resources/icudtl.dat"
#         "$<TARGET_FILE_DIR:Frontend>/icudtl.dat"
#     COMMAND ${CMAKE_COMMAND} -E copy
#         "${CEF_ROOT}/Resources/resources.pak"
#         "$<TARGET_FILE_DIR:Frontend>/resources.pak"
#     COMMAND ${CMAKE_COMMAND} -E copy
#         "${CEF_ROOT}/Resources/chrome_100_percent.pak"
#         "$<TARGET_FILE_DIR:Frontend>/chrome_100_percent.pak"
#     COMMAND ${CMAKE_COMMAND} -E copy
#         "${CEF_ROOT}/Resources/chrome_200_percent.pak"
#         "$<TARGET_FILE_DIR:Frontend>/chrome_200_percent.pak"
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#         "${CEF_ROOT}/Resources/locales"
#         "$<TARGET_FILE_DIR:Frontend>/locales"
#     COMMAND ${CMAKE_COMMAND} -E copy
#         "${CEF_ROOT}/Release/libcef.so"
#         "$<TARGET_FILE_DIR:Frontend>/libcef.so"
#     COMMAND ${CMAKE_COMMAND} -E copy
#         "${CEF_ROOT}/Release/snapshot_blob.bin"
#         "$<TARGET_FILE_DIR:Frontend>/snapshot_blob.bin"
#     COMMAND ${CMAKE_COMMAND} -E copy
#         "${CEF_ROOT}/Release/v8_context_snapshot.bin"
#         "$<TARGET_FILE_DIR:Frontend>/v8_context_snapshot.bin"
#     COMMENT "Copying CEF runtime files next to Frontend executable"
# )
