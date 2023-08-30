project "AtlasGeneratorTest"
kind "ConsoleApp"

language "C++"
cppdialect "C++17"

objdir "%{wks.location}/build/obj/%{cfg.buildcfg}/%{cfg.system}/%{cfg.architecture}/%{prj.name}"

targetdir "%{wks.location}/build/bin/%{cfg.buildcfg}/%{cfg.system}/%{cfg.architecture}/%{prj.name}"

files {
	"src/**.cpp",
	"%{wks.location}/include/ResourcePublisher/Writer/SWF/AtlasGenerator.h",
	"%{wks.location}/src/ResourcePublisher/Writer/SWF/AtlasGenerator.cpp"
}

includedirs {
	"%{wks.location}/include",
	"%{wks.location}/ThirdParty/OpenCV/include",
	"%{wks.location}/ThirdParty/libnest2d/include"
}

links {
	"%{wks.location}/ThirdParty/OpenCV/lib/%{cfg.buildcfg}/%{cfg.system}/*",
    "%{wks.location}/ThirdParty/libnest2d/lib/%{cfg.buildcfg}/%{cfg.architecture}/%{cfg.system}/*",
}

filter {"system:windows", "configurations:Debug"}
linkoptions {"/DEBUG:FASTLINK", "/IGNORE:4099"}

filter "configurations:Debug"
defines { "DEBUG" }
runtime "Debug"
symbols "on"

filter "configurations:Release"
defines { "NDEBUG" }
runtime "Release"
optimize "on"
