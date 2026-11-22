#pragma once

#include "AnimateModule.h"
#include "flash/flash.h"
#include "nlohmann/json.hpp"

#include <filesystem>
#include <string>
using namespace nlohmann;

namespace sc::Adobe {
    class SCConfig : public Animate::Publisher::GenericPublisherConfig {
    public:
        enum class Quality : uint8_t {
            Highest,
            High,
            Medium,
            Low
        };

        enum class SWFType : uint8_t {
            SC1,
            SC2
        };

    public:
        fs::path documentPath = "";
        fs::path outputFilepath = "";
        SWFType type = SWFType::SC2;

        bool backwardCompatibility = false;

        sc::flash::Signature compression = sc::flash::Signature::Zstandard;
        bool exportToExternal = false;
        fs::path exportToExternalPath = "";
        bool repackAtlas = true;
        // Defaults mirror PublisherDefaultSettings in packages/publisher/src/Publisher/PublisherSettings.ts.
        bool autoProperties = false;

        sc::flash::SWFTexture::TextureEncoding textureEncoding = sc::flash::SWFTexture::TextureEncoding::Raw;
        bool hasExternalTexture = true;
        bool hasExternalTextureFile = false;
        bool compressExternalTextureFile = true;
        bool hasLowresTexture = false;
        bool generateLowresTexture = false;
        bool hasMultiresTexture = false;
        std::string multiResolutionSuffix = "_highres";
        std::string lowResolutionSuffix = "_lowres";
        float textureScaleFactor = 1.f;
        Quality textureQuality = Quality::Highest;
        uint32_t textureMaxWidth = 4096;
        uint32_t textureMaxHeight = 4096;

        bool writeCustomProperties = true;
        bool hasPrecisionMatrices = false;
        bool lowPrecisionMatrices = false;
        bool useShortFrames = false;
        bool writeFieldsText = true;
        bool generateStreamingTexture = false;
        bool aggressiveRasterization = false;

        bool useMultiDocument = false;
        std::vector<fs::path> documentsPaths;

    public:
        virtual bool FromDict(const FCM::PIFCMDictionary dict) override;
        virtual void Reset() override;
        void Load(const FCM::PIFCMDictionary dict);
        void Normalize();
        void NormalizePath(fs::path& path);

    private:
        void ApplyDefaults();

    public:
        void DoConfigLogging();
    };
}