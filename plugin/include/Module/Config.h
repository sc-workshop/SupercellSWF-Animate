#pragma once

#include <string>

#include <filesystem>
namespace fs = std::filesystem;

#include "IFCMDictionary.h"
#include "SupercellFlash.h"
#include "Utils.h"

#include "json.hpp"

using namespace nlohmann;

namespace sc {
	namespace Adobe {
		enum class PublisherMethod : uint8_t {
			JSON,
			SWF
		};

		enum class ExportsMode : uint8_t {
			AllMovieclips,
			UnusedMovieclips
		};

		enum class Quality : uint8_t {
			Highest,
			High,
			Medium,
			Low
		};

		struct Config {
			fs::path output = "";
			PublisherMethod method = PublisherMethod::SWF;

			sc::CompressionSignature compression = sc::CompressionSignature::ZSTD;
			bool exportToExternal = false;
			fs::path exportToExternalPath = "";

			bool hasExternalTexture = 0;
			uint8_t textureScaleFactor = 1;
			Quality textureQuality = Quality::Highest;
			uint32_t textureMaxWidth = 2048;
			uint32_t textureMaxHeight = 2048;
			SWFTexture::TextureEncoding textureEncoding = SWFTexture::TextureEncoding::Raw;

			ExportsMode exportsMode = ExportsMode::AllMovieclips;

			bool filledShapeOptimization = true;
			bool hasPrecisionMatrices = false;

			static Config FromDict(const FCM::PIFCMDictionary dict);
		};
	}
}