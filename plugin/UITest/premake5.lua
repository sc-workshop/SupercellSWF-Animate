project "PluginUITest"
kind "ConsoleApp"

language "C++"
cppdialect "C++17"

objdir "%{wks.location}/build/obj/%{cfg.buildcfg}/%{cfg.system}/%{cfg.architecture}/%{prj.name}"

files {
	"../include/Ui/**.h",
	"../src/Ui/**.cpp",
    "Main.cpp"
}

removefiles { "../src/Ui/PluginUI.cpp", "../src/include/PluginUI.h" }

includedirs {
	"../ThirdParty/wxWidget/include",
	"../include/"
}

filter "system:windows"
includedirs { "../ThirdParty/wxWidget/include/msvc" }
libdirs { "../ThirdParty/wxWidget/lib/vc_x64_lib" }

filter "configurations:Debug"
defines { "DEBUG" }
runtime "Debug"
symbols "on"

filter "configurations:Release"
defines { "NDEBUG" }
runtime "Release"
optimize "on"
