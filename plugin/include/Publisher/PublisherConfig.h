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

			static PublisherConfig FromDict(const FCM::PIFCMDictionary dict) {
				PublisherConfig result;

				//Basic
				std::string output;
				Utils::ReadString(dict, PUBLISHER_OUTPUT, output);
				result.output = Utils::GetPath(output);

				std::string debugMode = "0";
				Utils::ReadString(dict, PUBLISHER_DEBUG, debugMode);
				if (!debugMode.empty() && debugMode == "1")
				{
					result.debug = true;
				}

				// SWF
				std::string hasTexture;
				Utils::ReadString(dict, PUBLISHER_SWF_HAS_TEXTURE, hasTexture);
				if (!hasTexture.empty() && hasTexture == "1")
				{
					result.hasTexture = true;
				}

				std::string compression;
				Utils::ReadString(dict, PUBLISHER_SWF_COMPRESSION, compression);
				if (compression == "LZHAM") {
					result.compression = sc::CompressionSignature::LZHAM;
				}
				else if (compression == "ZSTD") {
					result.compression = sc::CompressionSignature::ZSTD;
				}
				else {
					result.compression = sc::CompressionSignature::LZMA;
				}

				return result;
			}

		};
	}
}