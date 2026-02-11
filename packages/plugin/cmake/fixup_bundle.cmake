# ------------------------------------------------------------
# fixup_plugin_bundle.cmake
# ------------------------------------------------------------
# Usage:
#   cmake -DBUNDLE_DIR="..." -DCONFIG=Debug -P fixup_plugin_bundle.cmake
# ------------------------------------------------------------

cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED BUNDLE_DIR)
    message(FATAL_ERROR "BUNDLE_DIR not defined")
endif()

if(NOT DEFINED CONFIG)
    message(FATAL_ERROR "CONFIG not defined")
endif()

message(STATUS "Fixing up plugin bundle: ${BUNDLE_DIR}")
message(STATUS "Using config: ${CONFIG}")

# --- Paths ---
set(LIBS_DIR "${BUNDLE_DIR}/libs")
file(MAKE_DIRECTORY "${LIBS_DIR}")

# --- Find all dylibs in build/_deps for current config ---
message(STATUS "Trying to find dylibs at ${CMAKE_BINARY_DIR}")
file(GLOB_RECURSE DYLIBS
    "${CMAKE_BINARY_DIR}/../_deps/*-build/**/${CONFIG}/*.dylib"
    "${CMAKE_BINARY_DIR}/../_deps/*-build/${CONFIG}/*.dylib"
)

if(DYLIBS)
    message(STATUS "Found ${DYLIBS}")
else()
    message(WARNING "No dylibs found for ${CONFIG}")
endif()

# --- Function to copy/symlink dylib and fix install_name ---
function(copy_or_symlink_dylib dylib)
    get_filename_component(dylib_name ${dylib} NAME)
    set(target_path "${LIBS_DIR}/${dylib_name}")

    # Remove existing file if exists
    if(EXISTS "${target_path}")
        file(REMOVE "${target_path}")
    endif()

    # Debug: create symlink, Release: copy
    if(CONFIG STREQUAL "Debug")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink "${dylib}" "${target_path}"
        )
    else()
        file(COPY "${dylib}" DESTINATION "${LIBS_DIR}")
    endif()
endfunction()

# --- Loop over dylibs ---
foreach(dylib ${DYLIBS})
    copy_or_symlink_dylib(${dylib})
endforeach()

# --- Fix rpath for plugin binaries ---
file(GLOB PLUGIN_BINARIES "${BUNDLE_DIR}/*")
foreach(bin ${PLUGIN_BINARIES})
    # Check if Mach-O binary
    execute_process(
        COMMAND file "${bin}"
        OUTPUT_VARIABLE FILE_INFO
        ERROR_QUIET
    )
    string(FIND "${FILE_INFO}" "Mach-O" is_macho)
    if(is_macho GREATER -1)
        # Check if rpath already exists
        execute_process(
            COMMAND otool -l "${bin}"
            OUTPUT_VARIABLE BIN_INFO
        )
        string(FIND "${BIN_INFO}" "@loader_path/libs" has_rpath)
        if(has_rpath EQUAL -1)
            execute_process(
                COMMAND install_name_tool -add_rpath "@loader_path/libs" "${bin}"
            )
        endif()

        # Fix all dylib references inside this binary
        foreach(dylib ${DYLIBS})
            get_filename_component(dylib_name ${dylib} NAME)
            execute_process(
                COMMAND install_name_tool -change "${dylib}" "@loader_path/libs/${dylib_name}" "${bin}"
                RESULT_VARIABLE res
                ERROR_QUIET
            )
            if(NOT res EQUAL 0)
                message(STATUS "Skipping ${dylib} for ${bin}, maybe not referenced.")
            endif()
        endforeach()
    endif()
endforeach()

message(STATUS "Plugin bundle fixup done!")
