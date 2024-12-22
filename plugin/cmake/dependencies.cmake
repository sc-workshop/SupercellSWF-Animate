include(FetchContent)

set(WK_SERIALIZATION_JSON ON)

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

set(BUILD_ATLAS_GENERATOR_WITH_IMAGE_CODECS ON)
set(BUILD_ATLAS_GENERATOR_CLI OFF)
# AtlasGenerator
FetchContent_Declare(
    AtlasGenerator
    GIT_REPOSITORY https://github.com/sc-workshop/AtlasGenerator.git
    GIT_TAG dev
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