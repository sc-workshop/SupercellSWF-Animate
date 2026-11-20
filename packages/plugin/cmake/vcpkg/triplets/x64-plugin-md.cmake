set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_BUILD_TYPE release)

# Prebuilt Skia's static lib calls zlib through dllimport symbols (__imp_*).
if(PORT STREQUAL "zlib")
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()
