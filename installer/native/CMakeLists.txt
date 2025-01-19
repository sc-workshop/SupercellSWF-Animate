cmake_minimum_required(VERSION 3.21)

project(NativeInstaller)

include(FetchContent)

set(TARGET info)
file(GLOB_RECURSE SOURCES
    "source/*"
)

add_executable(${TARGET} ${SOURCES})
target_compile_features(${TARGET} PRIVATE cxx_std_17)

FetchContent_Declare(
    cpu_features
    GIT_REPOSITORY https://github.com/google/cpu_features.git
    GIT_TAG v0.9.0
)
FetchContent_MakeAvailable(cpu_features)

target_link_libraries(
    ${TARGET} PUBLIC
    CpuFeatures::cpu_features
)