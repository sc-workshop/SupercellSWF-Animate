# Dear ImGui - lightweight immediate-mode UI used by the publish progress window.
FetchContent_Declare(
    ImGui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.93.0
    GIT_SHALLOW TRUE
)
FetchContent_GetProperties(ImGui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(ImGui)
endif()

set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

if(WIN32)
    list(APPEND IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
    )
elseif(APPLE)
    enable_language(OBJCXX)
    list(APPEND IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/backends/imgui_impl_osx.mm
        ${imgui_SOURCE_DIR}/backends/imgui_impl_metal.mm
    )
endif()

add_library(ImGui STATIC ${IMGUI_SOURCES})
target_include_directories(ImGui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

if(WIN32)
    target_link_libraries(ImGui PUBLIC d3d11 dxgi)
elseif(APPLE)
    target_link_libraries(ImGui PUBLIC
        "-framework Metal"
        "-framework MetalKit"
        "-framework AppKit"
        "-framework QuartzCore"
    )
endif()
