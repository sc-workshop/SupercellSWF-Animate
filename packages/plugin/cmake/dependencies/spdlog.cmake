# Logging
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 12.1.0
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(fmt)

# spdlog
set(SPDLOG_FMT_EXTERNAL ON)
set(SPDLOG_BUILD_SHARED  ${BUILD_SHARED_LIBS})
set(SPDLOG_ENABLE_PCH ON)
find_package(spdlog REQUIRED)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.17.0
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(spdlog)