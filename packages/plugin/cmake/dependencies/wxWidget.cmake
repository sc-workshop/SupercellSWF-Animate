# wxWidgets
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
set(wxBUILD_SHARED ${BUILD_SHARED_LIBS})
set(wxBUILD_MONOLITHIC OFF)

set(wxUSE_GUI ON)
set(wxUSE_XML OFF)
set(wxUSE_NET OFF)
set(wxUSE_REGEX OFF)
set(wxUSE_MEDIA OFF)
set(wxUSE_WEBVIEW OFF)
set(wxUSE_STC OFF)
set(wxUSE_AUI OFF)
set(wxUSE_RIBBON OFF)
set(wxUSE_PROPGRID OFF)
set(wxUSE_RICHTEXT OFF)
set(wxUSE_OPENGL OFF)
set(wxUSE_SOCKETS OFF)
set(wxUSE_URL OFF)
set(wxUSE_HTML OFF)
set(wxUSE_PROTOCOL OFF)
set(wxUSE_FS_ARCHIVE OFF)
set(wxUSE_FS_INET OFF)
set(wxUSE_WXHTML_HELP OFF)
set(wxUSE_WXXRC OFF)
set(wxUSE_WXADV OFF)

find_package(wxWidgets QUIET COMPONENTS core base)
if (wxWidgets_FOUND)
    include(${wxWidgets_USE_FILE})

    add_library(wxWidgetsCore INTERFACE)
    target_link_libraries(
        wxWidgetsCore INTERFACE
        ${wxWidgets_LIBRARIES}
    )
else()
    FetchContent_Declare(
        wxWidgets
        GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
        GIT_TAG v3.2.9
        FIND_PACKAGE_ARGS
    )
    FetchContent_MakeAvailable(wxWidgets)
	
    message(STATUS "Using FetchContent to build wxWidgets")
	if (NOT TARGET wxWidgetsCore)
		add_library(wxWidgetsCore INTERFACE)
		target_link_libraries(
			wxWidgetsCore INTERFACE
			wx::core wx::base 
		)
	endif()
endif()

