#pragma once

#include <IFCMDictionary.h>
#include <Utils.h>

#include <string>

#include <filesystem>
namespace fs = std::filesystem;

#include <SupercellFlash.h>

#define PUBLISHER_OUTPUT "PublishSettings.SupercellSWF.output"
#define PUBLISHER_DEBUG "PublishSettings.SupercellSWF.debug"
#define PUBLISHER_SWF_HAS_TEXTURE "PublishSettings.SupercellSWF.hasTexture"
#define PUBLISHER_SWF_COMPRESSION "PublishSettings.SupercellSWF.compression"

namespace sc {
	namespace Adobe {
		struct PublisherConfig {
			// Basic
			fs::path output = "";
			bool debug = 0;

			// SWF
			bool hasTexture = 0;
			sc::CompressionSignature compression = sc::CompressionSignature::LZMA;

			static PublisherConfig FromDict(const FCM::PIFCMDictionary dict);
		};
	}
}