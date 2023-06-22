project "Plugin"
kind "SharedLib"

language "C++"
cppdialect "C++17"

objdir "%{wks.location}/build/obj/%{cfg.buildcfg}/%{cfg.system}/%{cfg.architecture}/%{prj.name}"

files {
    "src/**.cpp",
    "include/**.h"
}

includedirs {
    -- Animate SDK
    "include/AnimateSDK/app/include/interfaces/DOM",
    "include/AnimateSDK/app/include/common",
    "include/AnimateSDK/core/include/common",
    "include/AnimateSDK/app/include/interfaces",
    "include/AnimateSDK/core/include/interfaces",

    -- ScAnimate
    "include",
    "ThirdParty/AtlasGenerator/include",
	"ThirdParty/OpenCV/include",

    -- SC
    "ThirdParty/SC/dependencies/Bytestream",
    "ThirdParty/SC/dependencies/Compression/include",
    "ThirdParty/SC/include",
	
	-- GUI
	"ThirdParty/wxWidget/include"
}

links {
    -- SC
    "SupercellFlash",
    "SupercellCompression",
    "LZMA",
    "LZHAM",
    "Zstandard",

    -- ScAnimate libs
    "AtlasGenerator",

    -- ZIP

    -- FLA
}



filter {"system:windows", "configurations:Debug"}
links {
	"ThirdParty/OpenCV/lib/%{cfg.architecture}/%{cfg.system}/static/opencv_world470d"
}
targetdir "../dist/com.scwmake.SupercellSWF/Plugin/lib/win"

filter {"system:windows", "configurations:Release"}
links {
	"ThirdParty/OpenCV/lib/%{cfg.architecture}/%{cfg.system}/static/opencv_world470"
}
targetdir "bin/win/"

filter "system:windows"
includedirs { "ThirdParty/wxWidget/include/msvc" }
libdirs { "ThirdParty/wxWidget/lib/vc_x64_lib" }
defines { "_WINDOWS", "_WIN32", "_CRT_SECURE_NO_WARNINGS", "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS", "WIN32_LEAN_AND_MEAN" }
targetextension ".fcm"

filter "configurations:Debug"
defines { "DEBUG" }
runtime "Debug"
symbols "on"

filter "configurations:Release"
defines { "NDEBUG" }
runtime "Release"
optimize "on"
