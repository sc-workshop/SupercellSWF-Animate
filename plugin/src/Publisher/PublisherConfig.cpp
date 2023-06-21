#include "Publisher/PublisherConfig.h"

namespace sc {
	namespace Adobe {
		PublisherConfig PublisherConfig::FromDict(const FCM::PIFCMDictionary dict) {
			PublisherConfig result;

			string dataString;
			Utils::ReadString(dict, "SupercellSWF", dataString);

			if (dataString.empty()) {
				return result;
			}

			json data = json::parse(dataString);

			result.output = fs::path(Utils::ToUtf16(data["output"]));

			result.exportToExternal = data["exportToExternal"];
			result.exportToExternalPath = fs::path(Utils::ToUtf16(data["exportToExternalPath"]));

			result.hasExternalTexture = data["hasExternalTexture"];
			result.textureMaxWidth = data["textureMaxWidth"];
			result.textureMaxHeight = data["textureMaxHeight"];

			// Enums
			{
				uint8_t publishMethod = data["method"];
				switch (publishMethod)
				{
				case 0:
					result.method = PublisherMethod::JSON;
					break;
				case 1:
					result.method = PublisherMethod::SWF;
					break;
				default:
					break;
				}
			}

			{
				uint8_t compressionMethod = data["compressionMethod"];
				switch (compressionMethod)
				{
				case 0:
					result.compression = sc::CompressionSignature::LZMA;
					break;
				case 1:
					result.compression = sc::CompressionSignature::LZHAM;
					break;
				case 2:
					result.compression = sc::CompressionSignature::ZSTD;
					break;
				default:
					break;
				}
			}

			{
				uint8_t scaleFactor = data["textureScaleFactor"];
				switch (scaleFactor)
				{
				case 0:
					result.textureScaleFactor = 1;
					break;
				case 1:
					result.textureScaleFactor = 2;
					break;
				case 2:
					result.textureScaleFactor = 4;
					break;
				default:
					break;
				}
			}

			return result;
		}
	}
}