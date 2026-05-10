include(ExternalProject)

# TODO: Check if this work on MacOS

# Required tools
find_program(GIT_EXECUTABLE git REQUIRED)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Starting variables
if(DEFINED ENV{SKIA_ROOT})
    set(SKIA_ROOT $ENV{SKIA_ROOT})
else()
    set(SKIA_ROOT "${CMAKE_SOURCE_DIR}/skia")
endif()

set(SKIA_BUILD_DIR "${SKIA_ROOT}/out")
set(SKIA_DEBUG_DIR "${SKIA_BUILD_DIR}/Debug")
set(SKIA_RELEASE_DIR "${SKIA_BUILD_DIR}/Release")

# Per-arch dirs for macOS
set(SKIA_DEBUG_ARM64_DIR "${SKIA_BUILD_DIR}/Debug-arm64")
set(SKIA_DEBUG_X86_64H_DIR "${SKIA_BUILD_DIR}/Debug-x86_64h")
set(SKIA_DEBUG_X86_64_DIR "${SKIA_BUILD_DIR}/Debug-x86_64")

set(SKIA_RELEASE_ARM64_DIR "${SKIA_BUILD_DIR}/Release-arm64")
set(SKIA_RELEASE_X86_64H_DIR "${SKIA_BUILD_DIR}/Release-x86_64h")
set(SKIA_RELEASE_X86_64_DIR "${SKIA_BUILD_DIR}/Release-x86_64")

# Check for existence
set(SKIA_NEEDS_BUILD OFF)

if(NOT EXISTS "${SKIA_ROOT}")
    set(SKIA_NEEDS_BUILD ON)
endif()

if(NOT EXISTS "${SKIA_DEBUG_DIR}")
    set(SKIA_NEEDS_BUILD ON)
endif()

if(NOT EXISTS "${SKIA_RELEASE_DIR}")
    set(SKIA_NEEDS_BUILD ON)
endif()

# Fetching and build with ExternalProject
if(SKIA_NEEDS_BUILD)
    message(STATUS "Skia not found. Configuring...")
endif()

set(GN_EXECUTABLE "")
set(NINJA_EXECUTABLE "")
set(CONFIGURE_SCRIPT "")

set(GN_ARGS_DEBUG "")
set(GN_ARGS_RELEASE "")

set(GN_BASE_DEBUG_ARGS
    skia_use_no_png_encode=false
    skia_use_libpng_encode=true
    skia_use_libpng_decode=true
    skia_use_system_libpng=false
)
set(GN_BASE_RELEASE_ARGS
    skia_use_no_png_encode=true
    skia_use_libpng_encode=false
    skia_use_libpng_decode=true
    skia_use_system_libpng=false
)
set(GN_BASE_ARGS
    skia_use_no_webp_encode=true
    skia_use_no_jpeg_encode=true
    skia_use_lua=false
    skia_use_libwebp_decode=false
    skia_use_libwebp_encode=false
    skia_use_libjpeg_turbo_encode=false
    skia_use_libjpeg_turbo_decode=false
    skia_use_jpeg_gainmaps=false
    skia_use_icu=false
    skia_use_harfbuzz=false
    skia_use_expat=false
    skia_use_dng_sdk=false
    skia_enable_tools=false
    skia_enable_spirv_validation=false
    skia_enable_pdf=false
    skia_enable_metal_debug_info=false
    skia_enable_gpu_debug_layers=false
    skia_enable_fontmgr_win_gdi=false
    skia_enable_android_utils=false
)

if(WIN32)
    set(NINJA_EXECUTABLE "${SKIA_ROOT}/third_party/ninja/ninja.exe")
    set(GN_EXECUTABLE "${SKIA_ROOT}/bin/gn.exe")
    set(CONFIGURE_SCRIPT "${CMAKE_SOURCE_DIR}/cmake/dependencies/configure_skia.bat")
    set(GN_ARGS_DEBUG
        ${GN_BASE_ARGS} ${GN_BASE_DEBUG_ARGS}
        is_official_build=false
        is_debug=true
        is_component_build=true
        skia_use_gl=true
        skia_use_d3d12=true
    )
    set(GN_ARGS_RELEASE
        ${GN_BASE_ARGS} ${GN_BASE_RELEASE_ARGS}
        is_official_build=true
        is_debug=false
        is_component_build=false
        skia_use_gl=true
        skia_use_d3d12=true
    )
else()
    set(NINJA_EXECUTABLE "${SKIA_ROOT}/third_party/ninja/ninja")
    set(GN_EXECUTABLE "${SKIA_ROOT}/bin/gn")
    set(CONFIGURE_SCRIPT "${CMAKE_SOURCE_DIR}/cmake/dependencies/configure_skia.sh")
    set(GN_ARGS_DEBUG
        ${GN_BASE_ARGS} ${GN_BASE_DEBUG_ARGS}
        is_official_build=false is_debug=true
        is_component_build=true
        skia_use_metal=true
        skia_use_gl=false
        skia_use_vulkan=false
    )
    set(GN_ARGS_RELEASE
        ${GN_BASE_ARGS} ${GN_BASE_RELEASE_ARGS}
        is_official_build=true
        is_debug=false
        is_component_build=false
        skia_use_metal=true
        skia_use_gl=false
        skia_use_vulkan=false
    )
endif()

string(JOIN " " SKIA_ARGS_RELEASE ${GN_ARGS_RELEASE})
string(JOIN " " SKIA_ARGS_DEBUG ${GN_ARGS_DEBUG})

if(APPLE)
    ExternalProject_Add(
        skia_project
        PREFIX ${CMAKE_BINARY_DIR}/skia_build

        GIT_REPOSITORY https://skia.googlesource.com/skia
        GIT_TAG 05a03f99c74e0fa2d350f26a4c951560d229bbd0
        SOURCE_DIR ${SKIA_ROOT}

        CONFIGURE_COMMAND
        cmake -E env GIT_SYNC_DEPS_SKIP_EMSDK=1 ${Python3_EXECUTABLE} ${SKIA_ROOT}/tools/git-sync-deps
        COMMAND ${Python3_EXECUTABLE} ${SKIA_ROOT}/bin/fetch-ninja

        COMMAND ${CONFIGURE_SCRIPT} ${GN_EXECUTABLE} ${SKIA_ROOT} ${SKIA_DEBUG_ARM64_DIR} "${SKIA_ARGS_DEBUG}" Debug arm64
        COMMAND ${CONFIGURE_SCRIPT} ${GN_EXECUTABLE} ${SKIA_ROOT} ${SKIA_DEBUG_X86_64H_DIR} "${SKIA_ARGS_DEBUG}" Debug x86_64h
        COMMAND ${CONFIGURE_SCRIPT} ${GN_EXECUTABLE} ${SKIA_ROOT} ${SKIA_DEBUG_X86_64_DIR} "${SKIA_ARGS_DEBUG}" Debug x86_64

        COMMAND ${CONFIGURE_SCRIPT} ${GN_EXECUTABLE} ${SKIA_ROOT} ${SKIA_RELEASE_ARM64_DIR} "${SKIA_ARGS_RELEASE}" Release arm64
        COMMAND ${CONFIGURE_SCRIPT} ${GN_EXECUTABLE} ${SKIA_ROOT} ${SKIA_RELEASE_X86_64H_DIR} "${SKIA_ARGS_RELEASE}" Release x86_64h
        COMMAND ${CONFIGURE_SCRIPT} ${GN_EXECUTABLE} ${SKIA_ROOT} ${SKIA_RELEASE_X86_64_DIR} "${SKIA_ARGS_RELEASE}" Release x86_64

        BUILD_COMMAND
        ${NINJA_EXECUTABLE} -C ${SKIA_DEBUG_ARM64_DIR} skia -v
        COMMAND ${NINJA_EXECUTABLE} -C ${SKIA_DEBUG_X86_64H_DIR} skia -v
        COMMAND ${NINJA_EXECUTABLE} -C ${SKIA_DEBUG_X86_64_DIR} skia -v

        COMMAND ${NINJA_EXECUTABLE} -C ${SKIA_RELEASE_ARM64_DIR} skia -v
        COMMAND ${NINJA_EXECUTABLE} -C ${SKIA_RELEASE_X86_64H_DIR} skia -v
        COMMAND ${NINJA_EXECUTABLE} -C ${SKIA_RELEASE_X86_64_DIR} skia -v

        COMMAND ${CMAKE_COMMAND} -E make_directory ${SKIA_DEBUG_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SKIA_RELEASE_DIR}

        COMMAND lipo -create
        ${SKIA_DEBUG_ARM64_DIR}/libskia.a
        ${SKIA_DEBUG_X86_64H_DIR}/libskia.a
        ${SKIA_DEBUG_X86_64_DIR}/libskia.a
        -output ${SKIA_DEBUG_DIR}/libskia.a

        COMMAND lipo -create
        ${SKIA_RELEASE_ARM64_DIR}/libskia.a
        ${SKIA_RELEASE_X86_64H_DIR}/libskia.a
        ${SKIA_RELEASE_X86_64_DIR}/libskia.a
        -output ${SKIA_RELEASE_DIR}/libskia.a

        COMMAND lipo -info ${SKIA_DEBUG_DIR}/libskia.a
        COMMAND lipo -info ${SKIA_RELEASE_DIR}/libskia.a

        INSTALL_COMMAND ""
        UPDATE_COMMAND ""

        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON
    )
else()
    ExternalProject_Add(
        skia_project
        PREFIX ${CMAKE_BINARY_DIR}/skia_build

        GIT_REPOSITORY https://skia.googlesource.com/skia
        GIT_TAG 05a03f99c74e0fa2d350f26a4c951560d229bbd0
        SOURCE_DIR ${SKIA_ROOT}

        CONFIGURE_COMMAND
        cmake -E env GIT_SYNC_DEPS_SKIP_EMSDK=1 ${Python3_EXECUTABLE} ${SKIA_ROOT}/tools/git-sync-deps
        COMMAND ${Python3_EXECUTABLE} ${SKIA_ROOT}/bin/fetch-ninja

        COMMAND ${CONFIGURE_SCRIPT} ${GN_EXECUTABLE} ${SKIA_ROOT} ${SKIA_DEBUG_DIR} "${SKIA_ARGS_DEBUG}" Debug
        COMMAND ${CONFIGURE_SCRIPT} ${GN_EXECUTABLE} ${SKIA_ROOT} ${SKIA_RELEASE_DIR} "${SKIA_ARGS_RELEASE}" Release

        BUILD_COMMAND
        ${NINJA_EXECUTABLE} -C ${SKIA_DEBUG_DIR} skia -v
        COMMAND ${NINJA_EXECUTABLE} -C ${SKIA_RELEASE_DIR} skia -v

        INSTALL_COMMAND ""
        UPDATE_COMMAND ""

        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON
    )
endif()

# Creating CMake target
add_library(skia_release STATIC IMPORTED GLOBAL)
set_target_properties(skia_release PROPERTIES
    IMPORTED_CONFIGURATIONS "Release"
)

add_library(skia_debug SHARED IMPORTED GLOBAL)
set_target_properties(skia_debug PROPERTIES
    IMPORTED_CONFIGURATIONS "Debug"
)

if(WIN32)
    set_target_properties(skia_release PROPERTIES
        IMPORTED_LOCATION_RELEASE "${SKIA_RELEASE_DIR}/skia.lib"
    )

    set_target_properties(skia_debug PROPERTIES
        IMPORTED_LOCATION_DEBUG "${SKIA_DEBUG_DIR}/skia.dll"
        IMPORTED_IMPLIB_DEBUG "${SKIA_DEBUG_DIR}/skia.dll.lib"
    )
else()
    set_target_properties(skia_release PROPERTIES
        IMPORTED_LOCATION_RELEASE "${SKIA_RELEASE_DIR}/libskia.a"
    )
    set_target_properties(skia_debug PROPERTIES
        IMPORTED_LOCATION_DEBUG "${SKIA_DEBUG_DIR}/libskia.dylib"
    )
endif()

add_library(skia INTERFACE)
add_dependencies(skia skia_project)
target_link_libraries(
    skia INTERFACE
    $<$<CONFIG:Debug>:skia_debug> # Debug lib
    $<$<NOT:$<CONFIG:Debug>>:skia_release> # Release lib

    $<$<PLATFORM_ID:Darwin>: # MacOS Specific libs
    "-framework Metal"
    "-framework CoreGraphics"
    "-framework CoreText"
    "-framework Foundation"
    >
)
target_include_directories(skia INTERFACE "${SKIA_ROOT}")
target_compile_definitions(skia INTERFACE
    $<$<CONFIG:Debug>:SKIA_DLL>
)
