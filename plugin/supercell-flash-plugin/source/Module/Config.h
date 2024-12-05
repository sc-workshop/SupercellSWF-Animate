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

			bool backwardCompatibility = false;

			sc::flash::Signature compression = sc::flash::Signature::Zstandard;
			bool exportToExternal = false;
			fs::path exportToExternalPath = "";

			sc::flash::SWFTexture::TextureEncoding textureEncoding = sc::flash::SWFTexture::TextureEncoding::KhronosTexture;
			bool hasExternalTexture = false;
			bool hasExternalCompressedTexture = true;
			bool hasLowresTexture = false;
			bool hasMultiresTexture = false;
			std::string multiResolutionSuffix;
			std::string lowResolutionSuffix;
			uint8_t textureScaleFactor = 1;
			Quality textureQuality = Quality::Highest;
			uint32_t textureMaxWidth = 2048;
			uint32_t textureMaxHeight = 2048;

			bool writeCustomProperties = true;
			bool hasPrecisionMatrices = false;
		public:
			virtual void FromDict(const FCM::PIFCMDictionary dict) override;
			void Normalize();
		};
	}
}