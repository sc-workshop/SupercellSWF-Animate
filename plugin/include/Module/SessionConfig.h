#pragma once

#include <string>

#include <filesystem>
namespace fs = std::filesystem;

#include "AnimateSDK/app/DOM/IFLADocument.h"
#include "AnimateSDK/core/IFCMDictionary.h"
#include "SupercellFlash.h"

#include "nlohmann/json.hpp"
using namespace nlohmann;

namespace sc {
	namespace Adobe {
		enum class Quality : uint8_t {
			Highest,
			High,
			Medium,
			Low
		};

		class PluginSessionConfig {
		public:
			static PluginSessionConfig& Instance()
			{
				static PluginSessionConfig singleton;
				return singleton;
			}

			static void Clear()
			{
				Instance() = PluginSessionConfig();
			}

		private:
			PluginSessionConfig() {}
			~PluginSessionConfig() {}
			PluginSessionConfig(const PluginSessionConfig&);

			PluginSessionConfig& operator=(PluginSessionConfig&&) = default;

		public:
			// Current document
			DOM::PIFLADocument document = nullptr;

		public:
			fs::path outputFilepath = "";

			sc::SWFStream::Signature compression = sc::SWFStream::Signature::Zstandard;
			bool exportToExternal = false;
			fs::path exportToExternalPath = "";

			SWFTexture::TextureEncoding textureEncoding = SWFTexture::TextureEncoding::KhronosTexture;
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

			bool hasPrecisionMatrices = false;
		public:
			void FromDict(const FCM::PIFCMDictionary dict);
			void Normalize();
		};
	}
}