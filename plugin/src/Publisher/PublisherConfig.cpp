#include "Publisher/PublisherConfig.h"

#define PUBLISHER_OUTPUT "PublishSettings.SupercellSWF.output"
#define PUBLISHER_METHOD "PublishSettings.SupercellSWF.method"

#define PUBLISHER_SWF_COMPRESSION "PublishSettings.SupercellSWF.compression"

#define PUBLISHER_SWF_HAS_TEXTURE "PublishSettings.SupercellSWF.hasTexture"
#define PUBLISHER_SWF_TEXTURE_SCALE_FACTOR "PublishSettings.SupercellSWF.textureScaleFactor"
#define PUBLISHER_SWF_TEXTURE_MAX_WIDTH "PublishSettings.SupercellSWF.textureMaxWidth"
#define PUBLISHER_SWF_TEXTURE_MAX_HEIGHT "PublishSettings.SupercellSWF.textureMaxHeight"

namespace sc {
	namespace Adobe {
		PublisherConfig PublisherConfig::FromDict(const FCM::PIFCMDictionary dict) {
			PublisherConfig result;

			{
				std::string output;
				Utils::ReadString(dict, PUBLISHER_OUTPUT, output);
				result.output = fs::path(Utils::ToUtf16(output));
			}

			{
				std::string exportMethod = "SWF";
				if (Utils::ReadString(dict, PUBLISHER_METHOD, exportMethod)) {
					if (!exportMethod.empty())
					{
						if (exportMethod == "SWF") {
							result.method = PublisherMethod::SWF;
						}
						else if (exportMethod == "JSON") {
							result.method = PublisherMethod::JSON;
						}
					}
				}
			}

			{
				std::string compression;
				if (Utils::ReadString(dict, PUBLISHER_SWF_COMPRESSION, compression)) {
					if (compression == "LZHAM") {
						result.compression = sc::CompressionSignature::LZHAM;
					}
					else if (compression == "ZSTD") {
						result.compression = sc::CompressionSignature::ZSTD;
					}
					else {
						result.compression = sc::CompressionSignature::LZMA;
					}
				}
			}

			{
				bool hasTexture = false;
				if (Utils::ReadBoolean(dict, PUBLISHER_SWF_HAS_TEXTURE, hasTexture)) {
					result.hasTexture = hasTexture;
				}
			}

			{
				int textureMaxWidth = 0;
				if (Utils::ReadInteger(dict, PUBLISHER_SWF_TEXTURE_MAX_WIDTH, textureMaxWidth)) {
					result.textureMaxWidth = (uint32_t)textureMaxWidth;
				}
			}

			{
				int textureMaxHeight = 0;
				if (Utils::ReadInteger(dict, PUBLISHER_SWF_TEXTURE_MAX_HEIGHT, textureMaxHeight)) {
					result.textureMaxHeight = (uint32_t)textureMaxHeight;
				}
			}

			{
				int scaleFactor = 0;
				if (Utils::ReadInteger(dict, PUBLISHER_SWF_TEXTURE_SCALE_FACTOR, scaleFactor)) {
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
			}

			return result;
		}
	}
}