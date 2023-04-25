workspace "ScAnimate"
architecture "x64"

configurations {
    "Debug",
    "Release"
}

group "Main"
include "../"

group "Compression"
include "../ThirdParty/SC/external/lzma"
include "../ThirdParty/SC/external/lzham"
include "../ThirdParty/SC/external/zstd"

group "Libraries"
include "../ThirdParty/SC/SupercellBytestream"
include "../ThirdParty/SC/SupercellCompression"
include "../ThirdParty/SC/SupercellFlash"
include "../ThirdParty/libjson"
