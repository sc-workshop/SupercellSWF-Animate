# Blend2D

include(ExternalProject)

set(BLEND2D_STATIC TRUE)
FetchContent_Declare(
    blend2d
    URL https://blend2d.com/download/blend2d-0.21.2.zip
    URL_HASH SHA256=c56a4d1b466c8f5f87253643c7e00fb9b41fd1572d9dfbcf9df521b8abee64b0
)

# Blend2D is quite difficult build as universal build on MacOS so...
# We should build it manually
if (APPLE)

    # Just fetch for this case
    if(NOT blend2d_POPULATED)
        FetchContent_Populate(blend2d)
    endif()

    set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/blend2d/build)
    set(INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/blend2d/install)
    set(UNIVERSAL_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/blend2d/lib)
    file(MAKE_DIRECTORY ${UNIVERSAL_LIB_DIR})

    set(BUILD_CONFIGS Debug Release)
    set(BUILD_ARHCS x86_64 arm64)

    set(UNIVERSAL_LIBS)
    foreach(CONFIG IN LISTS BUILD_CONFIGS)
        set(BUILD_TARGETS)
        set(BUILD_LIBS)
        foreach(ARCH IN LISTS BUILD_ARHCS)
            set(TARGET_NAME blend2d_${ARCH}_${CONFIG})
            set(BUILD_ARCH_DIR ${BUILD_DIR}/${CONFIG}/${ARCH})
            file(MAKE_DIRECTORY ${BUILD_ARCH_DIR})

            set(TARGET_ARCH "")
            if (ARCH STREQUAL "x86_64" OR ARCH STREQUAL "x86_64h")
                set(TARGET_ARCH "x86_64")
            elseif(ARCH STREQUAL "arm64")
                set(TARGET_ARCH "AARCH64")
            endif()

            ExternalProject_Add(${TARGET_NAME}
                SOURCE_DIR ${blend2d_SOURCE_DIR}
                BINARY_DIR ${BUILD_ARCH_DIR}
                CMAKE_ARGS
                    ${BLEND2D_CMAKE_ARGS}
                    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
                    -DCMAKE_OSX_ARCHITECTURES=${ARCH}
                    -DBLEND2D_TARGET_ARCH=${TARGET_ARCH}
                    -DBLEND2D_STATIC=ON
                BUILD_COMMAND
                    ${CMAKE_COMMAND} --build . --config ${CONFIG}
                BUILD_ALWAYS 1
                INSTALL_COMMAND ""
            )

            set(LIB_PATH ${BUILD_ARCH_DIR}/${CONFIG}/libblend2d.a)

            list(APPEND BUILD_TARGETS ${TARGET_NAME})
            list(APPEND BUILD_LIBS ${LIB_PATH})
        endforeach()

        set(UNIVERSAL_LIB ${UNIVERSAL_LIB_DIR}/libblend2d_universal_${CONFIG}.a)
        add_custom_command(
            OUTPUT ${UNIVERSAL_LIB}
            COMMAND lipo -create ${BUILD_LIBS} -output ${UNIVERSAL_LIB}
            DEPENDS ${BUILD_LIBS} ${blend2d_SOURCE_DIR}
        )

        add_custom_target(blend2d_universal_${CONFIG} ALL
            DEPENDS ${UNIVERSAL_LIB}
        )
        add_dependencies(blend2d_universal_${CONFIG} 
            ${BUILD_TARGETS}
        )

        set(UNIVERSAL_LIBS_${CONFIG} ${UNIVERSAL_LIB})
    endforeach()

    add_library(blend2d STATIC IMPORTED GLOBAL)
    target_compile_definitions(
        blend2d INTERFACE
        BL_STATIC
    )

    set_target_properties(blend2d PROPERTIES
        IMPORTED_LOCATION_DEBUG ${UNIVERSAL_LIBS_Debug}
        IMPORTED_LOCATION_RELEASE ${UNIVERSAL_LIBS_Release}
        IMPORTED_LOCATION_MINSIZEREL ${UNIVERSAL_LIBS_Release}
        IMPORTED_LOCATION_RELWITHDEBINFO ${UNIVERSAL_LIBS_Release}
        INTERFACE_INCLUDE_DIRECTORIES ${blend2d_SOURCE_DIR}
    )

    add_library(blend2d::blend2d ALIAS blend2d)

else()
    FetchContent_MakeAvailable(blend2d)
endif()