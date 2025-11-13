# wxWidgets
set(wxBUILD_SHARED ${BUILD_SHARED_LIBS})
FetchContent_Declare(
    wxWidgets
    GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
    GIT_TAG v3.2.6
)
FetchContent_MakeAvailable(wxWidgets)
