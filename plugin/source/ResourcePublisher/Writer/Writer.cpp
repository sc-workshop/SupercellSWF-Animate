#include "ResourcePublisher/Writer/Writer.h"

namespace sc {
	namespace Adobe {
		SCWriter::SCWriter()
		{
			PluginSessionConfig& config = PluginSessionConfig::Instance();

			if (config.exportToExternal && !fs::exists(config.exportToExternalPath))
			{
				throw PluginException("TID_SWF_MISSING_EXTERNAL_FILE", config.exportToExternalPath.wstring().c_str());
			}
		}

		SharedMovieclipWriter* SCWriter::AddMovieclip(SymbolContext& symbol) {
			return new SCMovieclipWriter(*this, symbol);
		}

		SharedShapeWriter* SCWriter::AddShape(SymbolContext& symbol) {
			return new SCShapeWriter(*this, symbol);
		}

		void SCWriter::AddModifier(uint16_t id, MaskedLayerState type) {
			MovieClipModifier& modifier = swf.movieclip_modifiers.emplace_back();
			modifier.id = id;

			switch (type)
			{
			case MaskedLayerState::MASK_LAYER:
				modifier.type = MovieClipModifier::Type::Mask;
				break;
			case MaskedLayerState::MASKED_LAYERS:
				modifier.type = MovieClipModifier::Type::Masked;
				break;
			case MaskedLayerState::MASKED_LAYERS_END:
				modifier.type = MovieClipModifier::Type::Unmasked;
				break;
			default:
				break;
			}
		}

		void SCWriter::AddTextField(uint16_t id, TextFieldInfo object) {
			TextField& textfield = swf.textfields.emplace_back();

			std::string text = Localization::ToUtf8(object.text);
			std::string fontName = Localization::ToUtf8(object.fontName);

			textfield.id = id;

			textfield.text = SWFString(text);

			textfield.font_name = SWFString(fontName);
			textfield.font_color = *(uint32_t*)&object.fontColor;
			textfield.font_size = (uint8_t)object.fontSize;

			switch (object.style.alignment)
			{
			case DOM::FrameElement::AlignMode::ALIGN_MODE_CENTER:
				textfield.font_align = sc::TextField::Align::Center;
				break;
			case DOM::FrameElement::AlignMode::ALIGN_MODE_JUSTIFY:
				textfield.font_align = sc::TextField::Align::Justify;
				break;
			case DOM::FrameElement::AlignMode::ALIGN_MODE_LEFT:
				textfield.font_align = sc::TextField::Align::Left;
				break;
			case DOM::FrameElement::AlignMode::ALIGN_MODE_RIGHT:
				textfield.font_align = sc::TextField::Align::Right;
				break;
			default:
				break;
			}

			textfield.left = (int16_t)ceil(object.bound.topLeft.x);
			textfield.top = (int16_t)ceil(object.bound.topLeft.y);

			textfield.right = (int16_t)ceil(object.bound.bottomRight.x);
			textfield.bottom = (int16_t)ceil(object.bound.bottomRight.y);

			if (object.fontStyle != DOM::FrameElement::REGULAR_STYLE_STR) {
				if (object.fontStyle != DOM::FrameElement::ITALIC_STYLE_STR) {
					textfield.is_italic = true;
				}
				else if (object.fontStyle != DOM::FrameElement::BOLD_STYLE_STR) {
					textfield.is_bold = true;
				}
				else if (object.fontStyle != DOM::FrameElement::BOLD_ITALIC_STYLE_STR) {
					textfield.is_italic = true;
					textfield.is_bold = true;
				}
			}

			if (object.renderingMode.aaMode == DOM::FrameElement::AAMode::ANTI_ALIAS_MODE_DEVICE) {
				textfield.use_device_font = true;
			}

			textfield.is_outlined = object.isOutlined;
			textfield.outline_color = *(uint32_t*)&object.outlineColor;
			textfield.auto_kern = object.autoKern == FCM::Boolean(true);
			textfield.is_multiline =
				object.lineMode == DOM::FrameElement::LineMode::LINE_MODE_SINGLE ? false : true;
		}

		void SCWriter::AddExportName(uint16_t id, std::string name) {
			ExportName& exportName = swf.exports.emplace_back();
			exportName.id = id;
			exportName.name = SWFString(name);
		}

		void SCWriter::LoadExternal() {
			PluginSessionConfig& config = PluginSessionConfig::Instance();
			PluginContext& context = PluginContext::Instance();

			fs::path& filepath = config.exportToExternalPath;

			SupercellSWF base_swf;
			base_swf.load(filepath);

			uint16_t idOffset = 0;
			{
				for (MovieClip& object : base_swf.movieclips) {
					if (object.id > idOffset) {
						idOffset = object.id;
					}
				}

				for (MovieClipModifier& object : base_swf.movieclip_modifiers) {
					if (object.id > idOffset) {
						idOffset = object.id;
					}
				}

				for (TextField& object : base_swf.textfields) {
					if (object.id > idOffset) {
						idOffset = object.id;
					}
				}

				for (Shape& object : base_swf.shapes) {
					if (object.id > idOffset) {
						idOffset = object.id;
					}
				}
			}
			idOffset++;

			// Display object processing
			for (MovieClip& object : swf.movieclips) {
				object.id += idOffset;

				for (DisplayObjectInstance& bind : object.instances) {
					bind.id += idOffset;
				}

				object.bank_index += swf.matrixBanks.size();

				base_swf.movieclips.push_back(object);
			}

			for (MovieClipModifier& object : swf.movieclip_modifiers) {
				object.id += idOffset;

				base_swf.movieclip_modifiers.push_back(object);
			}

			for (TextField& object : swf.textfields) {
				object.id += idOffset;

				base_swf.textfields.push_back(object);
			}

			for (Shape& object : swf.shapes) {
				object.id += idOffset;

				for (ShapeDrawBitmapCommand& bitmap : object.commands) {
					bitmap.texture_index += swf.textures.size();
				}

				base_swf.shapes.push_back(object);
			}

			// Common resources processing
			for (MatrixBank& bank : swf.matrixBanks) {
				base_swf.matrixBanks.push_back(bank);
			}
			for (SWFTexture& texture : swf.textures) {
				base_swf.textures.push_back(texture);
			}
			for (ExportName& object : swf.exports) {
				object.id += idOffset;
				base_swf.exports.push_back(object);
			}

			swf = base_swf;
		}

		void SCWriter::FinalizeAtlas()
		{
			PluginSessionConfig& config = PluginSessionConfig::Instance();
			PluginContext& context = PluginContext::Instance();

			StatusComponent* status = context.window()->CreateStatusBar(
				context.locale.GetString("TID_STATUS_SPRITE_PACK")
			);

			std::vector<AtlasGeneratorItem> items;
			for (ShapeSymbol& shape : shapes) {
				for (Sprite& sprite : shape.sprites)
				{
					items.emplace_back(sprite.image);
				}
			}

			int itemCount = (int)items.size();
			status->SetRange(itemCount);

			AtlasGeneratorConfig atlasConfig;
			atlasConfig.maxSize = { config.textureMaxWidth, config.textureMaxHeight };
			atlasConfig.scaleFactor = config.textureScaleFactor;
			atlasConfig.progress = [&status, &itemCount](int value) {
				status->SetProgress(itemCount - value);
			};

			std::vector<cv::Mat> textures;
			AtlasGeneratorResult packageResult = AtlasGenerator::Generate(items, textures, atlasConfig);

			switch (packageResult)
			{
			case AtlasGeneratorResult::OK:
				break;

			case AtlasGeneratorResult::BAD_POLYGON:
				throw PluginException(
					"%ls %ls",
					context.locale.GetString("TID_SWF_ATLAS_GENERATOR_TYPE").c_str(),
					context.locale.GetString("TID_SWF_ATLAS_GENERATOR_BAD_POLYGON").c_str()
				);

			case AtlasGeneratorResult::TOO_MANY_IMAGES:
				throw PluginException(
					"%ls %ls",
					context.locale.GetString("TID_SWF_ATLAS_GENERATOR_TYPE").c_str(),
					context.locale.GetString("TID_SWF_ATLAS_GENERATOR_TOO_MANY_IMAGES").c_str()
				);

			case AtlasGeneratorResult::TOO_BIG_IMAGE:
				throw PluginException(
					"%ls %ls",
					context.locale.GetString("TID_SWF_ATLAS_GENERATOR_TYPE").c_str(),
					context.locale.GetString("TID_SWF_ATLAS_GENERATOR_TOO_BIG_SPRITES").c_str()
				);

			default:
				throw PluginException(
					"%ls %ls",
					context.locale.GetString("TID_SWF_ATLAS_GENERATOR_TYPE").c_str(),
					context.locale.GetString("TID_UNKNOWN_EXCEPTION").c_str()
				);
			}

			status->SetLabel(
				context.locale.GetString("TID_STATUS_SAVE")
			);

			uint16_t itemIndex = 0;
			for (uint32_t shapeIndex = 0; swf.shapes.size() > shapeIndex; shapeIndex++)
			{
				Shape& shape = swf.shapes[shapeIndex];
				ShapeSymbol& symbol = shapes[shapeIndex];

				for (uint32_t spriteIndex = 0; symbol.sprites.size() > spriteIndex; spriteIndex++)
				{
					ShapeDrawBitmapCommand& command = shape.commands.emplace_back();
					AtlasGeneratorItem& generatorItem = items[itemIndex];
					Sprite& sprite = symbol.sprites[spriteIndex];
					DOM::Utils::MATRIX2D& matrix = sprite.matrix;

					command.texture_index = generatorItem.textureIndex;

					// If bitmap is sprite
					if (sprite.contour.empty()) {
						while (generatorItem.polygon.size() <= 3)
						{
							generatorItem.polygon.push_back(generatorItem.polygon[generatorItem.polygon.size() - 1]);
						}

						for (auto point : generatorItem.polygon) {
							ShapeDrawBitmapCommandVertex& vertex = command.vertices.emplace_back();

							vertex.u = point.uv.first / (float)textures[generatorItem.textureIndex].cols;
							vertex.v = point.uv.second / (float)textures[generatorItem.textureIndex].rows;

							vertex.x = (matrix.a * point.xy.first) + (matrix.c * point.xy.second) + matrix.tx;
							vertex.y = (matrix.b * point.xy.first) + (matrix.d * point.xy.second) + matrix.ty;
						}
					}

					// If bitmap is filled shape triangle
					else {
						while (sprite.contour.size() <= 3) {
							sprite.contour.push_back(sprite.contour[sprite.contour.size() - 1]);
						}

						float u = generatorItem.polygon[0].uv.first / (float)textures[generatorItem.textureIndex].cols;
						float v = generatorItem.polygon[0].uv.second / (float)textures[generatorItem.textureIndex].rows;

						for (const Point2D& point : sprite.contour) {
							ShapeDrawBitmapCommandVertex& vertex = command.vertices.emplace_back();

							vertex.u = u;
							vertex.v = v;

							vertex.x = (matrix.a * point.x) + (-matrix.b * point.y) + matrix.tx;
							vertex.y = (-matrix.c * point.x) + (matrix.d * point.y) + matrix.ty;
						}
					}
					itemIndex++;
				}
			}

			for (cv::Mat& atlas : textures) {
				cv::cvtColor(atlas, atlas, cv::COLOR_BGRA2RGBA);

				SWFTexture& texture = swf.textures.emplace_back();
				size_t textureDataSize = atlas.total() * atlas.elemSize();
				MemoryStream textureData(atlas.ptr(), textureDataSize);
				texture.load_from_buffer(textureData, atlas.cols, atlas.rows, sc::SWFTexture::PixelFormat::RGBA8);

				if (config.textureEncoding == SWFTexture::TextureEncoding::Raw)
				{
					switch (config.textureQuality)
					{
					case Quality::Highest:
						texture.pixel_format(SWFTexture::PixelFormat::RGBA8);
						break;
					case Quality::High:
					case Quality::Medium:
						texture.pixel_format(SWFTexture::PixelFormat::RGBA4);
						break;
					case Quality::Low:
						texture.pixel_format(SWFTexture::PixelFormat::RGB5_A1);
						break;
					default:
						break;
					}
				}
				else
				{
					texture.encoding(SWFTexture::TextureEncoding::KhronosTexture);
				}
			}

			status->Destroy();
		}

		void SCWriter::Finalize() {
			PluginContext& context = PluginContext::Instance();
			PluginSessionConfig& config = PluginSessionConfig::Instance();

			enum FinalizeStep : uint8_t
			{
				INIT = 0,
				ATLAS_FINALIZE = 1,
				EXTERNAL_LOADING = 2,
				SWF_SAVING = 3,
				END = 4
			};

			StatusComponent* status = context.window()->CreateStatusBar(
				context.locale.GetString("TID_STATUS_INIT"),
				u"",
				END
			);

			status->SetProgress(ATLAS_FINALIZE);
			status->SetLabel(context.locale.GetString("TID_STATUS_TEXTURE_SAVE"));
			FinalizeAtlas();

			if (config.exportToExternal) {
				status->SetProgress(EXTERNAL_LOADING);
				status->SetLabel(context.locale.GetString("TID_EXTERNAL_FILE_LOAD"));
				LoadExternal();
			}

			swf.use_external_texture = config.hasExternalTexture;
			swf.use_low_resolution = config.hasLowresTexture;
			swf.use_multi_resolution = config.hasMultiresTexture;
			swf.multi_resolution_suffix = sc::SWFString(config.multiResolutionSuffix);
			swf.low_resolution_suffix = sc::SWFString(config.multiResolutionSuffix);
			swf.use_precision_matrix = config.hasPrecisionMatrices;
			swf.use_external_texture_files = config.hasExternalCompressedTexture;

			fs::path filepath = config.outputFilepath.replace_extension("sc");
			fs::path basename = filepath.filename();

			status->SetProgress(EXTERNAL_LOADING);
			status->SetLabel(
				context.locale.GetString(
					"TID_FILE_SAVE", basename.u16string().c_str()
				)
			);
			 
			swf.save(filepath, config.compression);

			status->Destroy();
		}
	}
}