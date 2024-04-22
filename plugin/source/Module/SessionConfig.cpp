#include "Module/SessionConfig.h"
#include "Module/PluginContext.h"
#include "PluginConfiguration.h"
#include "Module/Localization.h"
#include "Module/PluginException.h"

namespace sc {
	namespace Adobe {
		void PluginSessionConfig::FromDict(const FCM::PIFCMDictionary dict) {
			PluginContext& context = PluginContext::Instance();

			std::string serializedConfig;
			if (!dict->Get(DOCTYPE_NAME, serializedConfig))
			{
				context.logger->warn("Failed to load publish config for " DOCTYPE_NAME);
				return;
			};

			if (serializedConfig.empty()) {
				context.logger->warn("Publish settings is empty");
				return;
			}

			context.logger->info("Successfully loaded publish settings");

			context.logger->info("Publish Settings:");

			json data = json::parse(serializedConfig);

			outputFilepath = fs::path(Localization::ToUtf16(data["output"]));
			context.logger->info("	outputFilepath: {}", outputFilepath.string());

			hasPrecisionMatrices = data["hasPrecisionMatrices"];
			context.logger->info("	hasPrecisionMatrices: {}", hasPrecisionMatrices);

			hasPrecisionMatrices = data["writeCustomProperties"];
			context.logger->info("	writeCustomProperties: {}", writeCustomProperties);

			exportToExternal = data["exportToExternal"];
			context.logger->info("	exportToExternal: {}", exportToExternal);

			exportToExternalPath = fs::path(Localization::ToUtf16(data["exportToExternalPath"]));
			context.logger->info("	exportToExternalPath: {}", exportToExternalPath.string());

			hasExternalTexture = data["hasExternalTexture"];
			context.logger->info("	hasExternalTexture: ", hasExternalTexture);
			hasExternalCompressedTexture = data["hasExternalCompressedTexture"];
			context.logger->info("	hasExternalCompressedTexture: {}", hasExternalCompressedTexture);
			hasLowresTexture = data["hasLowresTexture"];
			context.logger->info("	hasLowresTexture: {}", hasLowresTexture);
			hasMultiresTexture = data["hasMultiresTexture"];
			context.logger->info("	hasMultiresTexture: {}", hasMultiresTexture);
			textureMaxWidth = data["textureMaxWidth"];
			context.logger->info("	textureMaxWidth: {}", textureMaxWidth);
			textureMaxHeight = data["textureMaxHeight"];
			context.logger->info("	textureMaxHeight: {}", textureMaxHeight);

			if (data["compressionMethod"].is_number_unsigned()) {
				compression = (sc::SWFStream::Signature)(data["compressionMethod"]);
				context.logger->info("	compression: {}", (uint8_t)compression);
			}

			if (data["textureScaleFactor"].is_number_unsigned()) {
				uint8_t scaleFactor = data["textureScaleFactor"];
				context.logger->info("	scaleFactor: {}", scaleFactor);
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
				context.logger->info("	textureEncoding: {}", (uint8_t)textureEncoding);
			}

			if (data["textureQuality"].is_number_unsigned()) {
				textureQuality = (Quality)data["textureQuality"];
				context.logger->info("	textureQuality: {}", (uint8_t)textureQuality);
			}

			if (data["multiResolutinSuffix"].is_string()) {
				multiResolutionSuffix = data["multiResolutinSuffix"];
				context.logger->info("	multiResolutinSuffix: {}", multiResolutionSuffix);
			}

			if (data["lowResolutionSuffix"].is_string()) {
				lowResolutionSuffix = data["lowResolutionSuffix"];
				context.logger->info("	lowResolutionSuffix: {}", lowResolutionSuffix);
			}
		}

		void PluginSessionConfig::Normalize()
		{
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
				else
				{
					outputFilepath = "File";
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

			if (exportToExternal && !exportToExternalPath.empty())
			{
				if (!documentPath.empty())
				{
					exportToExternalPath = (documentPath.parent_path() / exportToExternalPath).make_preferred();
				}
			}
		}
	}
}