cmake_minimum_required(VERSION 3.21)
include(FetchContent)

set(TARGET ScAnimatePlugin)
project(${TARGET})

option(SC_ANIMATE_IMAGE_DEBUG "SC Plugin Image Debug Mode" OFF)

# SC Flash
FetchContent_Declare(
    SupercellFlash
    GIT_REPOSITORY https://github.com/sc-workshop/SupercellFlash.git
    GIT_TAG dev
)
FetchContent_MakeAvailable(SupercellFlash)

if (${SC_ANIMATE_IMAGE_DEBUG})
	set(BUILD_LIST CACHE INTERNAL ""
        core
        imgproc
        imgcodecs
        highgui
    )
endif()

# AtlasGenerator
FetchContent_Declare(
    AtlasGenerator
    GIT_REPOSITORY https://github.com/sc-workshop/AtlasGenerator.git
    GIT_TAG main
)
FetchContent_MakeAvailable(AtlasGenerator)

if (${SC_ANIMATE_IMAGE_DEBUG})
	target_include_directories("AtlasGenerator" PUBLIC
		${OPENCV_MODULE_opencv_highgui_LOCATION}/include
	)
endif()

# wxWidgets
set(wxBUILD_SHARED ${BUILD_SHARED_LIBS})
FetchContent_Declare(
    wxWidgets
    GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
    GIT_TAG v3.2.4
)
FetchContent_MakeAvailable(wxWidgets)

# CDT
FetchContent_Declare(
    CDT
    GIT_REPOSITORY https://github.com/artem-ogre/CDT.git
    GIT_TAG 1.4.0
    SOURCE_SUBDIR CDT 
)
FetchContent_MakeAvailable(CDT)

# JSON
FetchContent_Declare(
    json 
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# spdlog
set(SPDLOG_BUILD_SHARED  ${BUILD_SHARED_LIBS})
set(SPDLOG_ENABLE_PCH ON)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(spdlog)

file(GLOB_RECURSE SOURCES
    "source/*"
)

file(GLOB_RECURSE HEADERS
    "include/*"
)

add_library(${TARGET} SHARED ${SOURCES} ${HEADERS})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})
sc_core_base_setup(${TARGET}) 

set_target_properties(${TARGET}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/animate_bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/animate_bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/animate_bin"
)

set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME "Plugin")
set_target_properties(${TARGET} PROPERTIES SUFFIX ".fcm")

target_compile_definitions(
	${TARGET} PRIVATE
	$<${SC_MSVC}: _CRT_SECURE_NO_WARNINGS _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING>
	$<$<BOOL:${SC_ANIMATE_IMAGE_DEBUG}>: CV_DEBUG >
)

target_link_libraries(${TARGET} PUBLIC 
    SupercellFlash
    AtlasGenerator
    CDT
    wx::core wx::base
    nlohmann_json::nlohmann_json
    spdlog::spdlog
	$<$<BOOL:${SC_ANIMATE_IMAGE_DEBUG}>: opencv_highgui>
)

target_include_directories(${TARGET}
    PUBLIC
    "include/"
)

if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(
        TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${TARGET}> $<TARGET_FILE_DIR:${TARGET}>
        COMMAND_EXPAND_LISTS
    )

endif()