project "libjson"
kind "StaticLib"

language "C++"
cppdialect "C++17"

targetdir "%{wks.location}/build/bin/%{cfg.buildcfg}/%{cfg.system}/%{cfg.architecture}/%{prj.name}"
objdir "%{wks.location}/build/obj/%{cfg.buildcfg}/%{cfg.system}/%{cfg.architecture}/%{prj.name}"

files {
  "src/**.cpp"
}

includedirs {
  "./",
  "src/"
}

filter "configurations:Debug"
defines { "DEBUG" }
runtime "Debug"
symbols "on"

filter "configurations:Release"
defines { "NDEBUG" }
runtime "Release"
optimize "on"
