workspace "ScAnimate"
architecture "x64"

configurations {
    "Debug",
    "Release"
}

include "premake5.lua"
include "UITest/"


group "Compression"
include "ThirdParty/SC/dependencies/Compression/dependencies/lzham"
include "ThirdParty/SC/dependencies/Compression/dependencies/lzma"
include "ThirdParty/SC/dependencies/Compression/dependencies/zstd"

group "Compression/Image"
include "ThirdParty/SC/dependencies/TextureLoader/ThirdParty/astc-encoder"
include "ThirdParty/SC/dependencies/TextureLoader/ThirdParty/ETCPACK"

group "Libraries"
include "ThirdParty/AtlasGenerator"
include "ThirdParty/SC/dependencies/Compression/"
include "ThirdParty/SC/"
include "ThirdParty/SC/dependencies/TextureLoader"

