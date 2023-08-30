workspace "ScAnimate"
architecture "x64"

configurations {
    "Debug",
    "Release"
}

include "./"

group "Compression"
include "ThirdParty/SC/dependencies/Compression/dependencies/lzham"
include "ThirdParty/SC/dependencies/Compression/dependencies/lzma"
include "ThirdParty/SC/dependencies/Compression/dependencies/zstd"

group "Compression/Image"
include "ThirdParty/SC/dependencies/TextureLoader/ThirdParty/astc-encoder"
include "ThirdParty/SC/dependencies/TextureLoader/ThirdParty/ETCPACK"
include "ThirdParty/SC/dependencies/TextureLoader/ThirdParty/libktx"
include "ThirdParty/SC/dependencies/TextureLoader/ThirdParty/basisu"
include "ThirdParty/SC/dependencies/TextureLoader/ThirdParty/dfdutils"

group "Libraries"
include "ThirdParty/SC/dependencies/Compression/"
include "ThirdParty/SC/"
include "ThirdParty/SC/dependencies/TextureLoader"
include "AtlasGenerator/"

filter "configurations:Debug"
optimize "off"
symbols "on"

filter "configurations:Release"
symbols "off"
optimize "Speed"
