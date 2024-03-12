#include "Module/SessionConfig.h"
#include "Module/PluginContext.h"
#include "PluginConfiguration.h"
#include "Module/Localization.h"
#include "Module/PluginException.h"

namespace sc {
	namespace Adobe {
		void PluginSessionConfig::FromDict(const FCM::PIFCMDictionary dict) {
			std::string serializedConfig;
			if (!dict->Get(DOCTYPE_NAME, serializedConfig))
			{
				return;
			};

			if (serializedConfig.empty()) {
				return;
			}

			json data = json::parse(serializedConfig);

			outputFilepath = fs::path(Localization::ToUtf16(data["output"]));
			fs::path documentPath;
			{
				FCM::StringRep16 documentPathPtr;
				document->GetPath(&documentPathPtr);
				PluginContext& context = PluginContext::Instance();
				if (documentPathPtr)
				{
					documentPath = fs::path((const char16_t*)documentPathPtr);
					context.falloc->Free(documentPathPtr);
				}
			}

			if (outputFilepath.empty())
			{
				if (!documentPath.empty())
				{
					outputFilepath = documentPath;
				}
			}
			else if (outputFilepath.is_relative())
			{
				if (!documentPath.empty())
				{
					outputFilepath = (documentPath.parent_path() / outputFilepath).make_preferred();
				}
			}

			if (outputFilepath.has_extension())
			{
				fs::path outputExt = outputFilepath.extension();
				if (outputExt.compare(".xfl") == 0)
				{
					outputFilepath = outputFilepath.parent_path();
				}
				else
				{
					outputFilepath.replace_extension();
				}
			}

			filledShapeOptimization = data["filledShapeOptimization"];
			hasPrecisionMatrices = data["hasPrecisionMatrices"];

			exportToExternal = data["exportToExternal"];
			exportToExternalPath = fs::path(Localization::ToUtf16(data["exportToExternalPath"]));

			if (exportToExternal && !exportToExternalPath.empty())
			{
				if (!documentPath.empty())
				{
					exportToExternalPath = (documentPath.parent_path() / exportToExternalPath).make_preferred();
				}
			}

			hasExternalTexture = data["hasExternalTexture"];
			hasExternalCompressedTexture = data["hasExternalCompressedTexture"];
			hasLowresTexture = data["hasLowresTexture"];
			hasMultiresTexture = data["hasMultiresTexture"];
			textureMaxWidth = data["textureMaxWidth"];
			textureMaxHeight = data["textureMaxHeight"];

			if (data["compressionMethod"].is_number_unsigned()) {
				compression = (sc::SWFStream::Signature)(data["compressionMethod"]);
			}

			if (data["textureScaleFactor"].is_number_unsigned()) {
				uint8_t scaleFactor = data["textureScaleFactor"];
				switch (scaleFactor)
				{
				case 0:
					textureScaleFactor = 1;
					break;
				case 1:
					textureScaleFactor = 2;
					break;
				case 2:
					textureScaleFactor = 4;
					break;
				default:
					break;
				}
			}

			if (data["textureEncoding"].is_number_unsigned()) {
				textureEncoding = (SWFTexture::TextureEncoding)data["textureEncoding"];
			}

			if (data["textureQuality"].is_number_unsigned()) {
				textureQuality = (Quality)data["textureQuality"];
			}

			if (data["multiResolutinSuffix"].is_string()) {
				multiResolutionSuffix = data["multiResolutinSuffix"];
			}

			if (data["multiResolutinSuffix"].is_string()) {
				multiResolutionSuffix = data["multiResolutinSuffix"];
			}

			if (data["lowResolutionSuffix"].is_string()) {
				lowResolutionSuffix = data["lowResolutionSuffix"];
			}
		}
	}
}