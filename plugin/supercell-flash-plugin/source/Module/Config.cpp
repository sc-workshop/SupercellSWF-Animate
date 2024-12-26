#include "Config.h"
#include "PluginConfiguration.h"
#include "Module/Module.h"

namespace sc {
	namespace Adobe {
		void SCConfig::FromDict(const FCM::PIFCMDictionary dict) {
			
			Load(dict);
			Normalize();
		}

		void SCConfig::Load(const FCM::PIFCMDictionary dict)
		{
			SCPlugin& context = SCPlugin::Instance();

			std::string serializedConfig;
			if (!dict->Get(DOCTYPE_NAME, serializedConfig))
			{
				context.logger->warn(std::string("Failed to load publish config for ").append(DOCTYPE_NAME));
				return;
			};

			if (serializedConfig.empty()) {
				context.logger->warn("Publish settings is empty");
				return;
			}

			context.logger->info("Successfully loaded publish settings");

			context.logger->info("Publish Settings:");

			json data = json::parse(serializedConfig);

			outputFilepath = fs::path(FCM::Locale::ToUtf16(data["output"]));
			context.logger->info("	outputFilepath: {}", outputFilepath.string());

			if (data.count("type"))
			{
				type = data["type"];
			}
			else
			{
				// Backward compatibility with old documents
				type = SWFType::SC1;
			}

			context.logger->info("	type: {}", (uint8_t)type);

			backwardCompatibility = data["backwardCompatibility"];
			context.logger->info("	backwardCompatibility: {}", backwardCompatibility);

			hasPrecisionMatrices = data["hasPrecisionMatrices"];
			context.logger->info("	hasPrecisionMatrices: {}", hasPrecisionMatrices);

			writeCustomProperties = data["writeCustomProperties"];
			context.logger->info("	writeCustomProperties: {}", writeCustomProperties);

			exportToExternal = data["exportToExternal"];
			context.logger->info("	exportToExternal: {}", exportToExternal);

			exportToExternalPath = fs::path(FCM::Locale::ToUtf16(data["exportToExternalPath"]));
			context.logger->info("	exportToExternalPath: {}", exportToExternalPath.string());

			hasExternalTexture = data["hasExternalTexture"];
			context.logger->info("	hasExternalTexture: {}", hasExternalTexture);
			hasExternalTextureFile = data["hasExternalTextureFile"];
			context.logger->info("	hasExternalTextureFile: {}", hasExternalTextureFile);
			compressExternalTextureFile = data["compressExternalTextureFile"];
			context.logger->info("	compressExternalTextureFile: {}", compressExternalTextureFile);
			hasLowresTexture = data["hasLowresTexture"];
			context.logger->info("	hasLowresTexture: {}", hasLowresTexture);
			hasMultiresTexture = data["hasMultiresTexture"];
			context.logger->info("	hasMultiresTexture: {}", hasMultiresTexture);
			textureMaxWidth = data["textureMaxWidth"];
			context.logger->info("	textureMaxWidth: {}", textureMaxWidth);
			textureMaxHeight = data["textureMaxHeight"];
			context.logger->info("	textureMaxHeight: {}", textureMaxHeight);

			if (data["compressionMethod"].is_number_unsigned()) {
				compression = (sc::flash::Signature)(data["compressionMethod"]);
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
				textureEncoding = (sc::flash::SWFTexture::TextureEncoding)data["textureEncoding"];
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

		void SCConfig::Normalize()
		{
			FCM::PluginModule& context = FCM::PluginModule::Instance();

			fs::path documentPath;
			{
				FCM::StringRep16 documentPathPtr;
				activeDocument->GetPath(&documentPathPtr);
				
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