workspace "ScAnimate"
architecture "x64"

configurations {
    "Debug",
    "Release"
}

include "premake5.lua"
include "UITest/"

group "SC"
include "ThirdParty/SC/dependencies/Compression/"
include "ThirdParty/SC/"

group "SC/Compression"
include "ThirdParty/SC/dependencies/Compression/dependencies/lzham"
include "ThirdParty/SC/dependencies/Compression/dependencies/lzma"
include "ThirdParty/SC/dependencies/Compression/dependencies/zstd"


group "Libraries"
include "ThirdParty/AtlasGenerator"

