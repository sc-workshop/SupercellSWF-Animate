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

#target_compile_definitions(
#	${TARGET} PRIVATE
#	$<${SC_MSVC}: _CRT_SECURE_NO_WARNINGS _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING>
#)

target_link_libraries(${TARGET} PUBLIC 
    wk::atlasGenerator
    wx::core wx::base
    supercell::flash
    spdlog::spdlog
    Adobe::Animate
    fmt::fmt
    CDT
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