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
    "include",
    "include/AnimateSDK/app/include/interfaces/DOM",
    "include/AnimateSDK/app/include/common",
    "include/AnimateSDK/core/include/common",
    "include/AnimateSDK/app/include/interfaces",
    "include/AnimateSDK/core/include/interfaces",
    "ThirdParty/libjson",
    "ThirdParty/SC/SupercellFlash/include",
    "ThirdParty/SC/SupercellBytestream/include",
    "ThirdParty/SC/SupercellCompression/include",
    "ThirdParty/AtlasGenerator/include",
}

links {
    "SupercellFlash",
    "SupercellCompression",
    "LZMA",
    "LZHAM",
    "Zstandard",
    "libjson",
    "AtlasGenerator"
}

filter {"system:windows", "configurations:Debug"}
links {
	"ThirdParty/AtlasGenerator/ThirdParty/lib/opencv/%{cfg.architecture}/%{cfg.system}/static/opencv_world470d"
}

filter {"system:windows", "configurations:Release"}
links {
	"ThirdParty/AtlasGenerator/ThirdParty/lib/opencv/%{cfg.architecture}/%{cfg.system}/static/opencv_world470"
}

filter "system:windows"
defines { "_WINDOWS" }
targetdir "project/win/"
targetextension ".fcm"

filter "system:macosx"
targetdir "project/mac/"

filter "configurations:Debug"
defines { "DEBUG" }
runtime "Debug"
symbols "on"

filter "configurations:Release"
defines { "NDEBUG" }
runtime "Release"
optimize "on"
