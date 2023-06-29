#include "Module/Config.h"

namespace sc {
	namespace Adobe {
		Config Config::FromDict(const FCM::PIFCMDictionary dict) {
			Config result;

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

			if (data["method"].is_number_unsigned()) {
				result.method = (PublisherMethod)data["method"];
			}

			if (data["compressionMethod"].is_number_unsigned()) {
				result.compression = (sc::CompressionSignature)(data["compressionMethod"] + 1);
			}

			if (data["textureScaleFactor"].is_number_unsigned()) {
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

			if (data["exportsMode"].is_number_unsigned()) {
				result.exportsMode = (ExportsMode)data["exportsMode"];
			}

			return result;
		}
	}
}