# Blend2D
FetchContent_Declare(
    blend2d
    URL https://blend2d.com/download/blend2d-0.11.5-all.tar.gz
    URL_HASH SHA256=a38d61e498981a3ea944b5a8a4d7e6895019fc610931a0e70328ccc2631fc8aa
)
FetchContent_MakeAvailable(blend2d)

set(BLEND2D_DIR ${blend2d_SOURCE_DIR}/blend2d)
set(BLEND2D_STATIC TRUE)
add_subdirectory(${BLEND2D_DIR})