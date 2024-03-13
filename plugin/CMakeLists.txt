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

# libnest2d
set(RP_ENABLE_DOWNLOADING ON)

FetchContent_Declare(
    libnest2d
    GIT_REPOSITORY https://github.com/tamasmeszaros/libnest2d.git
    GIT_TAG master
)
FetchContent_MakeAvailable(libnest2d)

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

# OpenCV
set(OPENCV_FORCE_3RDPARTY_BUILD ON)
set(WITH_OPENMP ON)
set(WITH_NVCUVENC OFF)
set(WITH_NVCUVID OFF)
set(WITH_FFMPEG OFF)
set(WITH_JASPER OFF)
set(WITH_OPENJPEG OFF)
set(WITH_JPEG OFF)
set(WITH_WEBP OFF)
set(WITH_TIFF OFF)
set(WITH_V4L OFF)
set(WITH_DSHOW OFF)
set(WITH_MSMF OFF)
set(WITH_DIRECTX OFF)
set(WITH_DIRECTML OFF)
set(WITH_OPENCL_D3D11_NV OFF)
set(WITH_PROTOBUF OFF)
set(WITH_IMGCODEC_PXM OFF)
set(WITH_IMGCODEC_PFM OFF)
set(WITH_QUIRC OFF)
set(BUILD_SHARED_LIBS OFF)
set(BUILD_opencv_apps OFF)
set(BUILD_opencv_js OFF)
set(BUILD_ANDROID_PROJECTS OFF)
set(BUILD_ANDROID_EXAMPLES OFF)
set(BUILD_DOCS OFF)
set(BUILD_PACKAGE OFF)
set(BUILD_JAVA OFF)
set(BUILD_LIST 
    core
    imgproc
    imgcodecs
)
FetchContent_Declare(
        opencv
        GIT_REPOSITORY https://github.com/opencv/opencv.git
        GIT_TAG 4.6.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(opencv)

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

set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME "Plugin")
set_target_properties(${TARGET} PROPERTIES SUFFIX ".fcm")

target_compile_definitions(
	${TARGET} PRIVATE
	$<${SC_MSVC}: _CRT_SECURE_NO_WARNINGS _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING>
)


target_link_libraries(${TARGET} PUBLIC 
    SupercellFlash
    libnest2d
    CDT
    opencv_core opencv_imgproc opencv_imgcodecs 
    wx::core wx::base
    nlohmann_json::nlohmann_json
)

target_include_directories(${TARGET}
    PUBLIC
    "include/"
    ${OPENCV_CONFIG_FILE_INCLUDE_DIR}
    ${opencv_SOURCE_DIR}/include
    ${OPENCV_MODULE_opencv_core_LOCATION}/include
    ${OPENCV_MODULE_opencv_imgproc_LOCATION}/include
    ${OPENCV_MODULE_opencv_imgcodecs_LOCATION}/include
)
