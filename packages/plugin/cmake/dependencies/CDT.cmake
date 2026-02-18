# CDT
FetchContent_Declare(
    CDT
    GIT_REPOSITORY https://github.com/artem-ogre/CDT.git
    GIT_TAG 1.4.4
    SOURCE_SUBDIR CDT 
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(CDT)

if (NOT TARGET CDT::CDT)
    add_library(CDT::CDT ALIAS CDT)
endif()