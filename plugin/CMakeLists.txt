cmake_minimum_required(VERSION 3.18)
include(FetchContent)

set(TARGET ScAnimatePlugin)
project(${TARGET})

# SC Flash
FetchContent_Declare(
    SupercellFlash
    GIT_REPOSITORY https://github.com/sc-workshop/SupercellFlash.git
    GIT_TAG dev
)
FetchContent_MakeAvailable(SupercellFlash)

# AtlasGenerator
FetchContent_Declare(
    AtlasGenerator
    GIT_REPOSITORY https://github.com/sc-workshop/AtlasGenerator.git
    GIT_TAG main
)
FetchContent_MakeAvailable(AtlasGenerator)

# wxWidgets
set(wxBUILD_SHARED OFF)
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
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME "Plugin")
set_target_properties(${TARGET} PROPERTIES SUFFIX ".fcm")

target_compile_definitions(
	${TARGET} PRIVATE
	$<${SC_MSVC}: _CRT_SECURE_NO_WARNINGS _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING>
)


target_link_libraries(${TARGET} PUBLIC 
    SupercellFlash
    AtlasGenerator
    CDT
    wx::core wx::base
    nlohmann_json::nlohmann_json
)

target_include_directories(${TARGET}
    PUBLIC
    "include/"
)
