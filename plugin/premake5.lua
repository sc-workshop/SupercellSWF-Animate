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
    "include/",
	"ThirdParty/",

    -- Animate SDK
    "include/AnimateSDK/app/include/interfaces/DOM/",
    "include/AnimateSDK/app/include/common/",
    "include/AnimateSDK/core/include/common/",
    "include/AnimateSDK/app/include/interfaces/",
    "include/AnimateSDK/core/include/interfaces/",

    -- Image processing
	"ThirdParty/OpenCV/include/",
    "ThirdParty/libnest2d/include/",

    -- SC
    "ThirdParty/SC/dependencies/Bytestream/",
    "ThirdParty/SC/dependencies/Compression/include/",
    "ThirdParty/SC/include/",
	
	-- UI
	"ThirdParty/wxWidget/include/",
	"ui/include/",
	
	-- Fills
	"ThirdParty/CDT/include/"
}

links {
    -- SC
	"SupercellFlash",
    "SupercellCompression",
	"LZMA",
	"LZHAM",
	"Zstandard",
	
    -- Textures
	"SupercellTextureLoader",
	"libktx",
	
	"ASTC",
	"dfdutils",
	"basisu",
	"ETCPACK",

    -- Image processing
    "ThirdParty/OpenCV/lib/%{cfg.buildcfg}/%{cfg.system}/*",
    "ThirdParty/libnest2d/lib/%{cfg.buildcfg}/%{cfg.architecture}/%{cfg.system}/*",
}


filter {"system:windows", "configurations:Debug"}
targetdir "../dist/com.scwmake.SupercellSWF/Plugin/lib/win"
linkoptions {"/DEBUG:FASTLINK", "/IGNORE:4099"}

filter {"system:windows", "configurations:Release"}
targetdir "bin/win/"


filter {"system:windows"}
includedirs { "ThirdParty/wxWidget/include/msvc" }
libdirs { "ThirdParty/wxWidget/lib/vc_x64_lib" }
defines { "_WINDOWS", "_WIN32", "_CRT_SECURE_NO_WARNINGS", "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS", "WIN32_LEAN_AND_MEAN" }
targetextension ".fcm"

filter "configurations:Debug"
runtime "Debug"
defines { "DEBUG", "_DEBUG" }

filter "configurations:Release"
runtime "Release"
defines { "NDEBUG" }
