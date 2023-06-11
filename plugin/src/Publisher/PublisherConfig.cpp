#include "Publisher/PublisherConfig.h"

namespace sc {
	namespace Adobe {
		PublisherConfig PublisherConfig::FromDict(const FCM::PIFCMDictionary dict) {
			PublisherConfig result;

			//Basic
			std::string output;
			Utils::ReadString(dict, PUBLISHER_OUTPUT, output);
			result.output = fs::path(Utils::ToUtf16(output));

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
	}
}