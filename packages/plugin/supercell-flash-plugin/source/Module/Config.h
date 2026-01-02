#pragma once

#include <string>

#include <filesystem>
#include "AnimateModule.h"

#include "flash/flash.h"

#include "nlohmann/json.hpp"
using namespace nlohmann;

namespace sc {
	namespace Adobe {
		class SCConfig : public Animate::Publisher::GenericPublisherConfig
		{
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
			bool autoProperties = true;

			sc::flash::SWFTexture::TextureEncoding textureEncoding = sc::flash::SWFTexture::TextureEncoding::KhronosTexture;
			bool hasExternalTexture = false;
			bool hasExternalTextureFile = true;
			bool compressExternalTextureFile = true;
			bool hasLowresTexture = false;
			bool generateLowresTexture = false;
			bool hasMultiresTexture = false;
			std::string multiResolutionSuffix;
			std::string lowResolutionSuffix;
			uint8_t textureScaleFactor = 1;
			Quality textureQuality = Quality::Highest;
			uint32_t textureMaxWidth = 4096;
			uint32_t textureMaxHeight = 4096;

			bool writeCustomProperties = true;
			bool hasPrecisionMatrices = false;
			bool lowPrecisionMatrices = false;
			bool useShortFrames = true;
			bool writeFieldsText = true;
			bool generateStreamingTexture = true;

			bool useMultiDocument = false;
			std::vector<fs::path> documentsPaths;
		public:
			virtual bool FromDict(const FCM::PIFCMDictionary dict) override;
			void Load(const FCM::PIFCMDictionary dict);
			void Normalize();
			void NormalizePath(fs::path& path);

			void DoConfigLogging();
		};
	}
}