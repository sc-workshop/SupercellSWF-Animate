set(TARGET ScAnimatePlugin)

file(GLOB_RECURSE SOURCES
    "source/*"
)

add_library(${TARGET} MODULE ${SOURCES})
wk_project_setup(${TARGET})

set_target_properties(${TARGET}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/animate_bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/animate_bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/animate_bin"
)

# Base properties
set_target_properties(${TARGET} PROPERTIES 
    OUTPUT_NAME "SupercellSWFAnimate"
    SUFFIX ".fcm"
)

target_link_libraries(${TARGET} PUBLIC 
    wk::atlasGenerator # Texture generate
    wxWidgetsCore      # Status bar
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

if (APPLE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    set(PRODUCT_NAME "SupercellSWFAnimate")
    set(BUNDLE_ID "SupercellSWFAnimate.fcm.plugin")

    set(VERSION_SHORT "1.4.0")
    set(VERSION_FULL  "1.4.0")

    set(COPYRIGHT "SC Workshop")
    set(MIN_OS "15.0")

    # Info plist configure
    configure_file(
        ${CMAKE_SOURCE_DIR}/cmake/Info.plist.in
        ${CMAKE_BINARY_DIR}/SupercellSWFAnimate.plist
        @ONLY
    )

    # Bundle properties
    set_target_properties(${TARGET} PROPERTIES
        MACOSX_BUNDLE TRUE
        BUNDLE TRUE
        BUNDLE_EXTENSION "fcm.plugin"
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_BINARY_DIR}/SupercellSWFAnimate.plist
    )

    # Binary properties
    set_target_properties(${TARGET} PROPERTIES
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${BUNDLE_ID}"
        XCODE_ATTRIBUTE_SKIP_INSTALL "NO"
        INSTALL_RPATH "@loader_path/libs"
    )

    # Add universal libs target for blend2d
    add_dependencies(${TARGET}
        blend2d_universal_Debug
        blend2d_universal_Release
    )
endif()

if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(
        TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${TARGET}> $<TARGET_FILE_DIR:${TARGET}>
        COMMAND_EXPAND_LISTS
    )
endif()