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

		SCWriter::~SCWriter()
		{
			if (fs::exists(sprite_temp_path)) {
				remove(sprite_temp_path);
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

		void SCWriter::AddTextField(uint16_t id, SymbolContext& symbol, TextElement& object) {
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

		void SCWriter::AddExportName(uint16_t id, const std::string& name) {
			ExportName& exportName = swf.exports.emplace_back();
			exportName.id = id;
			exportName.name = SWFString(name);
		}

		void SCWriter::LoadExternal() {
			PluginSessionConfig& config = PluginSessionConfig::Instance();

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

		void SCWriter::ProcessCommandTransform(
			ShapeDrawBitmapCommand& command,
			AtlasGenerator::Item::Transformation& transform,
			GraphicItem& item
		)
		{
			Matrix2x3<float> matrix = item.transformation();

			for (ShapeDrawBitmapCommandVertex& vertex : command.vertices)
			{
				Point<uint16_t> uv(vertex.u, vertex.v);
				Point<float> xy(vertex.x, vertex.y);
				transform.transform_point(uv);

				vertex.u = uv.u / (float)swf.textures[command.texture_index].image()->width();
				vertex.v = uv.v / (float)swf.textures[command.texture_index].image()->height();

				vertex.x = (matrix.a * xy.x) + (matrix.c * xy.y) + matrix.tx;
				vertex.y = (matrix.b * xy.x) + (matrix.d * xy.y) + matrix.ty;
			}
		}

		void SCWriter::ProcessSpriteItem(
			Shape& shape,
			AtlasGenerator::Item& atlas_item,
			SpriteItem& sprite_item
		)
		{
			ShapeDrawBitmapCommand& shape_command = shape.commands.emplace_back();
			Matrix2x3<float> matrix = sprite_item.transformation();

			shape_command.texture_index = atlas_item.texture_index;

			for (AtlasGenerator::Vertex& vertex : atlas_item.vertices)
			{
				ShapeDrawBitmapCommandVertex& shape_vertex = shape_command.vertices.emplace_back();

				shape_vertex.x = vertex.xy.x;
				shape_vertex.y = vertex.xy.y;
				shape_vertex.u = vertex.uv.u;
				shape_vertex.v = vertex.uv.v;
			}

			ProcessCommandTransform(shape_command, atlas_item.transform, sprite_item);
		}

		void SCWriter::ProcessFilledItem(
			Shape& shape,
			AtlasGenerator::Item& atlas_item,
			FilledItem& filled_item
		)
		{
			if (!atlas_item.is_rectangle() || atlas_item.vertices.size() != 4)
			{
				// Just in case if something wrong with item
				return;
			}

			const AtlasGenerator::Vertex& atlas_point = atlas_item.vertices[0];

			for (const FilledItemContour& contour : filled_item.contours)
			{
				ShapeDrawBitmapCommand& shape_command = shape.commands.emplace_back();

				for (const Point2D& point : contour.Contour())
				{
					ShapeDrawBitmapCommandVertex& shape_vertex = shape_command.vertices.emplace_back();

					shape_vertex.u = atlas_point.uv.x;
					shape_vertex.v = atlas_point.uv.y;
					shape_vertex.x = point.x;
					shape_vertex.y = point.y;

					ProcessCommandTransform(shape_command, atlas_item.transform, filled_item);
				}
			}
		}

		void SCWriter::FinalizeAtlas()
		{
			PluginSessionConfig& config = PluginSessionConfig::Instance();
			PluginContext& context = PluginContext::Instance();

			StatusComponent* status = context.Window()->CreateStatusBar(
				context.locale.GetString("TID_STATUS_SPRITE_PACK")
			);

			std::vector<Ref<AtlasGenerator::Item>> items;

			for (GraphicGroup& group : m_graphic_groups)
			{
				for (size_t i = 0; group.size() > i; i++)
				{
					GraphicItem& item = group.getItem(i);

					Ref<AtlasGenerator::Item>& atlas_item = items.emplace_back();

					if (item.IsSprite())
					{
						SpriteItem& sprite_item = *(SpriteItem*)&item;

						atlas_item = CreateRef<AtlasGenerator::Item>(sprite_item.image());
					}
					else if (item.IsFilledShape())
					{
						FilledItem& filled_item = *(FilledItem*)&item;
						atlas_item = CreateRef<AtlasGenerator::Item>(filled_item.Color());
					}
					else
					{
						throw PluginException("Not implemented");
					}
				}
			}

			AtlasGenerator::Config generator_config(
				AtlasGenerator::Config::TextureType::RGBA,
				config.textureMaxWidth,
				config.textureMaxHeight,
				config.textureScaleFactor,
				2
			);

			int itemCount = (int)items.size();
			status->SetRange(itemCount);

			generator_config.progress = [&status, &itemCount](int value) {
				status->SetProgress(itemCount - value);
			};

			AtlasGenerator::Generator generator(generator_config);

			uint16_t texture_count = 0;

			try
			{
				texture_count = generator.generate(items);
			}
			catch (const AtlasGenerator::PackagingException& exception)
			{
				throw PluginException("TODO");
			}

			for (uint16_t i = 0; texture_count > i; i++) {
				cv::Mat& atlas = generator.get_atlas(i);

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

			uint16_t command_index = 0;
			for (uint32_t shape_index = 0; swf.shapes.size() > shape_index; shape_index++)
			{
				Shape& shape = swf.shapes[shape_index];
				GraphicGroup& group = m_graphic_groups[shape_index];

				for (uint32_t group_item_index = 0; group.size() > group_item_index; group_item_index++)
				{
					AtlasGenerator::Item& atlas_item = *items[command_index];
					GraphicItem& item = group.getItem(group_item_index);

					if (item.IsSprite())
					{
						SpriteItem& sprite_item = *(SpriteItem*)&item;
						ProcessSpriteItem(
							shape, atlas_item, sprite_item
						);
					}
					else if (item.IsFilledShape())
					{
						FilledItem& filled_item = *(FilledItem*)&item;
						ProcessFilledItem(
							shape, atlas_item, filled_item
						);
					}

					command_index++;
				}
			}

			context.Window()->DestroyStatusBar(status);
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

			StatusComponent* status = context.Window()->CreateStatusBar(
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

			context.Window()->DestroyStatusBar(status);
		}
	}
}