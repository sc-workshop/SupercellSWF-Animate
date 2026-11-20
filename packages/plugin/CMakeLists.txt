cmake_minimum_required(VERSION 3.21)

project(AnimatePlugin)

# The progress UI is a standalone window on both supported platforms. wxWidgets used to own the
# universal-binary setting; keep it at top level now that wx is gone.
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
endif()

include(cmake/dependencies.cmake)

add_subdirectory(supercell-flash-plugin)