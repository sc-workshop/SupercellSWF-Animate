set(TARGET ScAnimatePlugin)

file(GLOB_RECURSE SOURCES
    "source/*"
)

add_library(${TARGET} SHARED ${SOURCES})
wk_project_setup(${TARGET})

set_target_properties(${TARGET}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/animate_bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/animate_bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/animate_bin"
)

set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME "Plugin")
set_target_properties(${TARGET} PROPERTIES SUFFIX ".fcm")

target_link_libraries(${TARGET} PUBLIC 
    wk::atlasGenerator # Texture generate
    wx::core wx::base  # Status bar
    supercell::flash   # File writer
    blend2d::blend2d   # Vector rasterizer
    spdlog::spdlog     # Logging
    Adobe::Animate     # Adobe Animate API
    fmt::fmt           # String format
    CDT                # Vector triangulation
    zip                # Fla reader
)

target_include_directories(${TARGET}
    PUBLIC
    "source/"
)

if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(
        TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${TARGET}> $<TARGET_FILE_DIR:${TARGET}>
        COMMAND_EXPAND_LISTS
    )

endif()