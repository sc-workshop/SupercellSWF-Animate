#pragma once

#include <IFCMDictionary.h>
#include <Utils.h>

#include <string>

#include <filesystem>
namespace fs = std::filesystem;

#include "JSON.hpp"
#include <SupercellFlash.h>

using namespace nlohmann;

namespace sc {
	namespace Adobe {
		enum class PublisherMethod : uint8_t {
			JSON,
			SWF
		};

		enum class PublisherMode : uint8_t {
			Import,
			Export
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
			PublisherMode mode = PublisherMode::Export;

			fs::path output = "";
			PublisherMethod method = PublisherMethod::SWF;

			sc::CompressionSignature compression = sc::CompressionSignature::LZMA;
			bool exportToExternal = false;
			fs::path exportToExternalPath = "";

			bool hasExternalTexture = 0;
			uint8_t textureScaleFactor = 1;
			Quality textureQuality = Quality::Highest;
			uint32_t textureMaxWidth = 2048;
			uint32_t textureMaxHeight = 2048;
			SWFTexture::TextureEncoding textureEncoding = SWFTexture::TextureEncoding::Raw;

			ExportsMode exportsMode = ExportsMode::AllMovieclips;

			static Config FromDict(const FCM::PIFCMDictionary dict);
		};
	}
}