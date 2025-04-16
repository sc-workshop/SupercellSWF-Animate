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
			SCConfig(
				Animate::DOM::PIFLADocument document, 
				const FCM::PIFCMDictionary settings, 
				const FCM::PIFCMDictionary publishSettings
			) : Animate::Publisher::GenericPublisherConfig(document, settings, publishSettings)
			{
			}

		public:
			fs::path outputFilepath = "";
			SWFType type = SWFType::SC2;

			bool backwardCompatibility = false;

			sc::flash::Signature compression = sc::flash::Signature::Zstandard;
			bool exportToExternal = false;
			fs::path exportToExternalPath = "";
			bool repackAtlas = true;

			sc::flash::SWFTexture::TextureEncoding textureEncoding = sc::flash::SWFTexture::TextureEncoding::KhronosTexture;
			bool hasExternalTexture = false;
			bool hasExternalTextureFile = true;
			bool compressExternalTextureFile = true;
			bool hasLowresTexture = false;
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
		public:
			virtual void FromDict(const FCM::PIFCMDictionary dict) override;
			void Load(const FCM::PIFCMDictionary dict);
			void Normalize();
		};
	}
}