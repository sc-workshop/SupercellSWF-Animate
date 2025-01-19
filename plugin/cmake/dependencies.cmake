include(FetchContent)

# SC Flash
FetchContent_Declare(
    SupercellFlash
    GIT_REPOSITORY https://github.com/sc-workshop/SupercellFlash.git
    GIT_TAG main
)
FetchContent_MakeAvailable(SupercellFlash)

# AnimateSDK
FetchContent_Declare(
    AnimateSDK
    GIT_REPOSITORY https://github.com/sc-workshop/AnimateSDK.git
    GIT_TAG main
)
FetchContent_MakeAvailable(AnimateSDK)

# AtlasGenerator
FetchContent_Declare(
    AtlasGenerator
    GIT_REPOSITORY https://github.com/sc-workshop/AtlasGenerator.git
    GIT_TAG main
)
FetchContent_MakeAvailable(AtlasGenerator)

# wxWidgets
set(wxBUILD_SHARED ${BUILD_SHARED_LIBS})
FetchContent_Declare(
    wxWidgets
    GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
    GIT_TAG v3.2.6
)
FetchContent_MakeAvailable(wxWidgets)

# CDT
FetchContent_Declare(
    CDT
    GIT_REPOSITORY https://github.com/artem-ogre/CDT.git
    GIT_TAG 1.4.1
    SOURCE_SUBDIR CDT 
)
FetchContent_MakeAvailable(CDT)

# Logging
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.0.2
)
FetchContent_MakeAvailable(fmt)

# spdlog
set(SPDLOG_FMT_EXTERNAL ON)
set(SPDLOG_BUILD_SHARED  ${BUILD_SHARED_LIBS})
set(SPDLOG_ENABLE_PCH ON)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.0
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
    blend2d
    URL https://blend2d.com/download/blend2d-0.11.5-all.tar.gz
    URL_HASH SHA256=a38d61e498981a3ea944b5a8a4d7e6895019fc610931a0e70328ccc2631fc8aa
)
FetchContent_MakeAvailable(blend2d)

set(BLEND2D_DIR ${blend2d_SOURCE_DIR}/blend2d)
set(BLEND2D_STATIC TRUE)
add_subdirectory(${BLEND2D_DIR})