#include "Config.h"
#include "PluginConfiguration.h"
#include "Module/Module.h"

namespace sc {
	namespace Adobe {
		bool SCConfig::FromDict(const FCM::PIFCMDictionary dict) {
			Load(dict);
			Normalize();

			return true;
		}

		void SCConfig::Load(const FCM::PIFCMDictionary dict)
		{
			using sc::flash::SWFTexture;

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

			json data = json::parse(serializedConfig);

			context.logger->info("Successfully loaded publish settings");

			outputFilepath = fs::path(FCM::Locale::ToUtf16(data["output"]));
			if (data.count("type"))
			{
				type = data["type"];
			}
			else
			{
				// Backward compatibility with old documents
				type = SWFType::SC1;
			}

			backwardCompatibility = data["backwardCompatibility"];

			hasPrecisionMatrices = data["hasPrecisionMatrices"];
			writeCustomProperties = data["writeCustomProperties"];

			exportToExternal = data["exportToExternal"];
			exportToExternalPath = fs::path(FCM::Locale::ToUtf16(data["exportToExternalPath"]));
			repackAtlas = data["repackAtlas"];
			autoProperties = data["autoProperties"];

			hasExternalTexture = data["hasExternalTexture"];
			hasExternalTextureFile = data["hasExternalTextureFile"];
			compressExternalTextureFile = data["compressExternalTextureFile"];
			
			textureMaxWidth = data["textureMaxWidth"];
			textureMaxHeight = data["textureMaxHeight"];

			if (data["compressionMethod"].is_number_unsigned()) {
				compression = (sc::flash::Signature)(data["compressionMethod"]);
			}

			if (data["textureScaleFactor"].is_number_unsigned()) {
				uint8_t scaleFactor = data["textureScaleFactor"];
				switch (scaleFactor)
				{
				case 0:
					textureScaleFactor = 1;
					break;
				case 1:
					textureScaleFactor = 1.5;
					break;
				case 2:
					textureScaleFactor = 2;
					break;
				case 3:
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

			// Disable lowres/highres generating for SCTX
			if (textureEncoding != SWFTexture::TextureEncoding::SupercellTexture) {
				hasLowresTexture = data["hasLowresTexture"];
				generateLowresTexture = data["generateLowresTexture"];
				hasMultiresTexture = data["hasMultiresTexture"];
			}
			
			if (data["multiResolutinSuffix"].is_string()) {
				multiResolutionSuffix = data["multiResolutinSuffix"];
			}

			if (data["lowResolutionSuffix"].is_string()) {
				lowResolutionSuffix = data["lowResolutionSuffix"];
			}

			lowPrecisionMatrices = data["lowPrecisionMatrices"];
			useShortFrames = data["useShortFrames"];
			writeFieldsText = data["writeFieldsText"];
			generateStreamingTexture = data["generateStreamingTexture"];
			useMultiDocument = data["multipleDocuments"];

			if (data["documentsPaths"].is_array()) {
				const auto& paths = data["documentsPaths"];
				for (const std::string& path : paths) {
					documentsPaths.push_back(path);
				}
			}

			if (!autoProperties) // If use autoProperties, do logging later, after all properties are set
			{
				DoConfigLogging();
			}
		}

		void SCConfig::Normalize()
		{
			FCM::PluginModule& context = FCM::PluginModule::Instance();

			documentPath = context.falloc->GetString16(
				activeDocument,
				&Animate::DOM::IFLADocument::GetPath
			);

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

			if (exportToExternal)
				NormalizePath(exportToExternalPath);

			for (auto& docPath : documentsPaths) {
				NormalizePath(docPath);
			}
		}

		void SCConfig::NormalizePath(fs::path& path)
		{
			if (path.empty())
				return;

			if (path.is_relative() && !documentPath.empty()) {
				path = (documentPath.parent_path() / path).make_preferred();
			}

		}

		void SCConfig::DoConfigLogging()
		{
			SCPlugin& context = SCPlugin::Instance();

			context.logger->info("Publish Settings:");
			
			context.logger->info("	documentPath: {}", documentPath.string());
			context.logger->info("	outputFilepath: {}", outputFilepath.string());
			context.logger->info("	type: {}", (uint8_t)type);
			context.logger->info("	backwardCompatibility: {}", backwardCompatibility);
			context.logger->info("	hasPrecisionMatrices: {}", hasPrecisionMatrices);
			context.logger->info("	writeCustomProperties: {}", writeCustomProperties);
			context.logger->info("	exportToExternal: {}", exportToExternal);
			context.logger->info("	exportToExternalPath: {}", exportToExternalPath.string());

			context.logger->info("	repackAtlas: {}", repackAtlas);
			context.logger->info("	autoProperties: {}", autoProperties);

			context.logger->info("	hasExternalTexture: {}", hasExternalTexture);
			context.logger->info("	hasExternalTextureFile: {}", hasExternalTextureFile);
			context.logger->info("	compressExternalTextureFile: {}", compressExternalTextureFile);
			context.logger->info("	hasLowresTexture: {}", hasLowresTexture);
			context.logger->info("	generateLowresTexture: {}", generateLowresTexture);
			context.logger->info("	hasMultiresTexture: {}", hasMultiresTexture);
			context.logger->info("	textureMaxWidth: {}", textureMaxWidth);
			context.logger->info("	textureMaxHeight: {}", textureMaxHeight);
			context.logger->info("	generateStreamingTexture: {}", generateStreamingTexture);

			context.logger->info("	compression: {}", (uint8_t)compression);

			context.logger->info("	scaleFactor: {}", textureScaleFactor);
			context.logger->info("	textureEncoding: {}", (uint8_t)textureEncoding);
			context.logger->info("	textureQuality: {}", (uint8_t)textureQuality);
			context.logger->info("	multiResolutinSuffix: {}", multiResolutionSuffix);
			context.logger->info("	lowResolutionSuffix: {}", lowResolutionSuffix);

			context.logger->info("	lowPrecisionMatrices: {}", lowPrecisionMatrices);
			context.logger->info("	useShortFrames: {}", useShortFrames);
			context.logger->info("	writeFieldsText: {}", writeFieldsText);
		}
	}
}