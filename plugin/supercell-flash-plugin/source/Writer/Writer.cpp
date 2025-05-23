#include "Writer.h"

#include "Module/Module.h"
#include "MovieclipWriter.h"
#include "ShapeWriter.h"
#include "TextFieldWriter.h"
#include "Module/SCPluginException.h"

#include "atlas_generator/Generator.h"
#include "atlas_generator/PackagingException.h"
#include "core/stb/stb.h"

#include "Reassemble/Object.hpp"
#include "Reassemble/Atlas.h"

using namespace Animate::Publisher;

namespace sc {
	namespace Adobe {
		SCWriter::SCWriter()
		{
		}

		SCWriter::~SCWriter()
		{
			if (fs::exists(sprite_temp_path))
			{
				fs::remove(sprite_temp_path);
			}
		}

		SharedMovieclipWriter* SCWriter::AddMovieclip(SymbolContext& symbol) {
			return new SCMovieclipWriter(*this, symbol);
		}

		SharedShapeWriter* SCWriter::AddShape(SymbolContext& symbol) {
			return new SCShapeWriter(*this, symbol);
		}

		void SCWriter::AddModifier(uint16_t id, MaskedLayerState type) {
			flash::MovieClipModifier& modifier = swf.movieclip_modifiers.emplace_back();
			modifier.id = id;

			switch (type)
			{
			case MaskedLayerState::MASK_LAYER:
				modifier.type = flash::MovieClipModifier::Type::Mask;
				break;
			case MaskedLayerState::MASKED_LAYERS:
				modifier.type = flash::MovieClipModifier::Type::Masked;
				break;
			case MaskedLayerState::MASKED_LAYERS_END:
				modifier.type = flash::MovieClipModifier::Type::Unmasked;
				break;
			default:
				break;
			}
		}

		SharedTextFieldWriter* SCWriter::AddTextField(Animate::Publisher::SymbolContext& symbol)
		{
			return new SCTextFieldWriter(*this, symbol);
		}

		uint16_t SCWriter::LoadExternal(fs::path path) {
			using namespace Animate::DOM;
			swf.load(path);

			{
				InputFileStream file(path);
				if (flash::SupercellSWF::IsSC2(file))
				{
					for (auto& shape : swf.shapes)
					{
						for (auto& command : shape.commands)
						{
							command.sort_advanced_vertices();
						}
					}
				}
			}

			texture_offset = swf.textures.size();
			uint16_t idOffset = 0;
			{
				for (flash::MovieClip& object : swf.movieclips) {
					if (object.id > idOffset) {
						idOffset = object.id;
					}
				}

				for (flash::MovieClipModifier& object : swf.movieclip_modifiers) {
					if (object.id > idOffset) {
						idOffset = object.id;
					}
				}

				for (flash::TextField& object : swf.textfields) {
					if (object.id > idOffset) {
						idOffset = object.id;
					}
				}

				for (flash::Shape& object : swf.shapes) {
					if (object.id > idOffset) {
						idOffset = object.id;
					}
				}
			}

			return ++idOffset;
		}

		void SCWriter::ProcessDrawCommand(
			flash::ShapeDrawBitmapCommand& command,
			wk::AtlasGenerator::Item::Transformation& transform,
			GraphicItem& item
		)
		{
			using namespace wk;
			using namespace wk::AtlasGenerator;

			wk::Matrix2D matrix = item.Transformation2D();
			flash::SWFTexture& texture = swf.textures[command.texture_index];
			if (command.vertices.empty()) return;

			// �opy the last vertex until size equals 4, this is important
			while (4 > command.vertices.size())
			{
				command.vertices.emplace_back(command.vertices[command.vertices.size() - 1]);
			}

			for (flash::ShapeDrawBitmapCommandVertex& vertex : command.vertices)
			{
				PointUV uv(vertex.u, vertex.v);
				PointF xy(vertex.x, vertex.y);
				transform.transform_point(uv);

				vertex.u = uv.u / (float)texture.image()->width();
				vertex.v = uv.v / (float)texture.image()->height();

				vertex.x = (matrix.a * xy.x) + (matrix.c * xy.y) + matrix.tx;
				vertex.y = (matrix.b * xy.x) + (matrix.d * xy.y) + matrix.ty;
			}
		}

		void SCWriter::ProcessVertices(
			flash::Shape& shape,
			const wk::AtlasGenerator::Container<wk::AtlasGenerator::Vertex>& vertices,
			wk::AtlasGenerator::Item& atlas_item,
			GraphicItem& item
		)
		{
			using namespace wk;
			using namespace AtlasGenerator;

			flash::ShapeDrawBitmapCommand& shape_command = shape.commands.emplace_back();
			shape_command.texture_index = atlas_item.texture_index + texture_offset;

			for (const Vertex& vertex : vertices)
			{
				auto& shape_vertex = shape_command.vertices.emplace_back();

				shape_vertex.x = vertex.xy.x;
				shape_vertex.y = vertex.xy.y;
				shape_vertex.u = vertex.uv.u;
				shape_vertex.v = vertex.uv.v;
			}

			ProcessDrawCommand(shape_command, atlas_item.transform, item);
		}

		void SCWriter::ProcessSpriteItem(
			flash::Shape & shape,
			wk::AtlasGenerator::Item& atlas_item,
			BitmapItem& sprite_item
		)
		{
			ProcessVertices(shape, atlas_item.vertices, atlas_item, sprite_item);
		}

		void SCWriter::ProcessSlicedItem(
			flash::Shape& shape,
			wk::AtlasGenerator::Item& atlas_item,
			SlicedItem& sliced_item
		)
		{
			using namespace wk;
			using namespace wk::AtlasGenerator;

			Item::Transformation transform(
				0,
				sliced_item.Translation()
			);

			Container<Container<Vertex>> regions;
			atlas_item.get_9slice(
				sliced_item.Guides(),
				regions, transform
			);

			for (const Container<Vertex>& region : regions)
			{
				ProcessVertices(shape, region, atlas_item, sliced_item);
			}

			for (auto& command : shape.commands)
			{
				for (auto& vertex : command.vertices)
				{
					vertex.x = std::floor(vertex.x);
					vertex.y = std::floor(vertex.y);
				}
			}
		}

		void SCWriter::ProcessFilledItem(
			flash::Shape& shape,
			wk::AtlasGenerator::Item& atlas_item,
			FilledItem& filled_item
		)
		{
			if (!atlas_item.get_colorfill().has_value()) return;

			auto atlas_point = atlas_item.get_colorfill().value();

			for (const FilledItemContour& contour : filled_item.contours)
			{
				flash::ShapeDrawBitmapCommand& shape_command = shape.commands.emplace_back();
				shape_command.texture_index = atlas_item.texture_index + texture_offset;

				for (const Point2D& point : contour.Contour())
				{
					flash::ShapeDrawBitmapCommandVertex& shape_vertex = shape_command.vertices.emplace_back();

					shape_vertex.u = atlas_point.uv.x;
					shape_vertex.v = atlas_point.uv.y;
					shape_vertex.x = point.x;
					shape_vertex.y = point.y;
				}

				ProcessDrawCommand(shape_command, atlas_item.transform, filled_item);
			}
		}

		void SCWriter::FinalizeAtlas()
		{
			using namespace wk;

			const SCConfig& config = SCPlugin::Publisher::ActiveConfig();
			SCPlugin& context = SCPlugin::Instance();

			StatusComponent* status = context.Window()->CreateStatusBarComponent(
				context.locale.GetString("TID_STATUS_SPRITE_PACK")
			);

			std::vector<AtlasGenerator::Item> items;

			for (GraphicGroup& group : m_graphic_groups)
			{
				for (size_t i = 0; group.Size() > i; i++)
				{
					GraphicItem& item = (GraphicItem&)group[i];

					if (item.IsSprite())
					{
						BitmapItem& sprite_item = (BitmapItem&)item;

						auto& atlas_item = items.emplace_back(
							sprite_item.Image(),
							item.Is9Sliced()
						);

						if (sprite_item.IsRasterizedVector() || item.Is9Sliced())
						{
							// Rasterized sprites already has premultiplied alpha so no need to preprocess it
							atlas_item.mark_as_preprocessed();
						}
					}
					else if (item.IsSolidColor())
					{
						FilledItem& filled_item = (FilledItem&)item;

						items.emplace_back(filled_item.Color());
					}
					else
					{
						throw Exception("Not implemented");
					}
				}
			}

			AtlasGenerator::Config generator_config(
				config.textureMaxWidth,
				config.textureMaxHeight,
				1.f / config.textureScaleFactor,
				2
			);

			int itemCount = (int)items.size();
			status->SetRange(itemCount);

			generator_config.progress = [&status](uint32_t value) {
				status->SetProgress(value);
			};

			AtlasGenerator::Generator generator(generator_config);

			uint16_t texture_count = 0;

			try
			{
				texture_count = generator.generate(items);
			}
			catch (const AtlasGenerator::PackagingException& exception)
			{
				// [AtlasGenerator] Reason / symbol name
				// or in case of unknown exception just reason
				if (exception.reason() == AtlasGenerator::PackagingException::Reason::Unknown)
				{
					throw SCPluginException(
						Localization::Format(
							u"[AtlasGenerator] %ls",
							context.locale.GetString("TID_SWF_ATLAS_UNKNOWN").c_str()
						)
					);
				}

				std::u16string reason;
				std::u16string symbol_name;

				switch (exception.reason())
				{
				case AtlasGenerator::PackagingException::Reason::UnsupportedImage:
					reason = context.locale.GetString("TID_SWF_ATLAS_UNSUPORTED_IMAGE");
					break;
				case AtlasGenerator::PackagingException::Reason::InvalidPolygon:
					reason = context.locale.GetString("TID_SWF_ATLAS_INVALID_POLYGON");
					break;
				case AtlasGenerator::PackagingException::Reason::TooBigImage:
					reason = context.locale.GetString("TID_SWF_ATLAS_TOO_BIG_IMAGE");
					break;
				default:
					break;
				}

				if (exception.index() != SIZE_MAX)
				{
					size_t atlas_item_index = 0;
					size_t group_index = 0;
					for (; m_graphic_groups.size() >= group_index; group_index++)
					{
						GraphicGroup& group = m_graphic_groups[group_index];
						for (size_t group_item_index = 0; group.Size() > group_item_index; group_item_index++)
						{
							if (atlas_item_index == exception.index())
							{
								symbol_name = m_graphic_groups[group_index][group_item_index].Symbol().name;
								goto FINALIZE_THROW;
							};

							atlas_item_index++;
						}
					}
				
					
				}
				else
				{
					symbol_name = context.locale.GetString("TID_SWF_ATLAS_UNKNOWN_SYMBOL");
				}

			FINALIZE_THROW:
				throw SCPluginException(
					Localization::Format(
						u"[AtlasGenerator] %ls %ls", reason.c_str(), symbol_name.c_str()
					)
				);
			}

			context.Window()->DestroyStatusBar(status);

			for (uint16_t i = 0; texture_count > i; i++) {
				wk::RawImage& atlas = generator.get_atlas(i);

				flash::SWFTexture& texture = swf.textures.emplace_back();
				texture.load_from_image(atlas);
			}

			uint16_t command_index = 0;
			uint16_t shape_index = (swf.shapes.size() - m_graphic_groups.size());
			for (uint32_t group_index = 0; m_graphic_groups.size() > group_index; group_index++, shape_index++)
			{
				flash::Shape& shape = swf.shapes[shape_index];
				GraphicGroup& group = m_graphic_groups[group_index];

				for (uint32_t group_item_index = 0; group.Size() > group_item_index; group_item_index++)
				{
					AtlasGenerator::Item& atlas_item = items[command_index];
					GraphicItem& item = (GraphicItem&)group[group_item_index];

					if (item.IsSprite())
					{
						if (item.Is9Sliced())
						{
							SlicedItem& sliced_item = *(SlicedItem*)&item;
							ProcessSlicedItem(
								shape, atlas_item, sliced_item
							);
						}
						else
						{
							BitmapItem& sprite_item = *(BitmapItem*)&item;
							ProcessSpriteItem(
								shape, atlas_item, sprite_item
							);
						}
					}
					else if (item.IsSolidColor() && atlas_item.get_colorfill().has_value())
					{
						FilledItem& filled_item = *(FilledItem*)&item;
						ProcessFilledItem(
							shape, atlas_item, filled_item
						);
					}

					command_index++;
				}
			}

			if (config.exportToExternal && config.repackAtlas)
			{
				flash::repack_atlas(swf);
			}

			wk::parallel::enumerate(
				swf.textures.begin(),
				swf.textures.end(),
				[&config](flash::SWFTexture& texture, size_t)
				{
					if (config.textureEncoding == flash::SWFTexture::TextureEncoding::Raw)
					{
						if (texture.image()->base_type() == Image::BasePixelType::RGBA)
						{
							switch (config.textureQuality)
							{
							case SCConfig::Quality::Highest:
								texture.pixel_format(flash::SWFTexture::PixelFormat::RGBA8);
								break;
							case SCConfig::Quality::High:
							case SCConfig::Quality::Medium:
								texture.pixel_format(flash::SWFTexture::PixelFormat::RGBA4);
								break;
							case SCConfig::Quality::Low:
								texture.pixel_format(flash::SWFTexture::PixelFormat::RGB5_A1);
								break;
							default:
								break;
							}
						}
					}
					else
					{
						texture.encoding(flash::SWFTexture::TextureEncoding::KhronosTexture);
					}
				}
			);

			if (config.type == SCConfig::SWFType::SC2)
			{
				for (auto& shape : swf.shapes)
				{
					for (flash::ShapeDrawBitmapCommand& command : shape.commands)
					{
						command.sort_advanced_vertices(true);
					}
				}
			}
		}

		void SCWriter::Finalize() {
			const SCConfig& config = SCPlugin::Publisher::ActiveConfig();
			SCPlugin& context = SCPlugin::Instance();

			enum FinalizeStep : uint8_t
			{
				INIT = 0,
				ATLAS_FINALIZE = 1,
				SWF_SAVING = 2,
				END = 3
			};

			StatusComponent* status = context.Window()->CreateStatusBarComponent(
				context.locale.GetString("TID_STATUS_INIT"),
				u"",
				END
			);

			if (config.exportToExternal)
			{
				flash::remove_unused(swf);
			}

			status->SetProgress(ATLAS_FINALIZE);
			status->SetStatusLabel(context.locale.GetString("TID_STATUS_TEXTURE_SAVE"));
			FinalizeAtlas();

			swf.use_external_texture = config.hasExternalTexture;
			swf.use_low_resolution = config.hasLowresTexture;
			swf.use_multi_resolution = config.hasMultiresTexture;
			swf.multi_resolution_suffix = flash::SWFString(config.multiResolutionSuffix);
			swf.low_resolution_suffix = flash::SWFString(config.lowResolutionSuffix);
			swf.use_precision_matrix = config.hasPrecisionMatrices;
			swf.save_custom_property = config.writeCustomProperties;

			// SC2 settings
			{
				flash::Sc2CompileSettings& sc2 = swf.sc2_compile_settings;
				sc2.use_half_precision_matrices = config.lowPrecisionMatrices;
				sc2.use_short_frames = config.useShortFrames;
			}

			// Raw textures can be stored only inside texture files
			if (config.textureEncoding == flash::SWFTexture::TextureEncoding::Raw)
			{
				swf.use_external_textures = false;
			}
			else
			{
				swf.use_external_textures = config.hasExternalTextureFile;
				swf.compress_external_textures = config.compressExternalTextureFile;
			}

			fs::path filepath = fs::path(config.outputFilepath).replace_extension("sc");
			fs::path basename = filepath.filename();

			status->SetProgress(SWF_SAVING);
			status->SetStatusLabel(
				context.locale.GetString(
					"TID_FILE_SAVE", basename.u16string().c_str()
				)
			);

			switch (config.type)
			{
			case SCConfig::SWFType::SC1:
				swf.save(filepath, config.compression, config.generateLowresTexture);
				break;
			case SCConfig::SWFType::SC2:
				swf.save_sc2(filepath);
				break;
			default:
				break;
			}

			context.Window()->DestroyStatusBar(status);
		}

		wk::RawImageRef SCWriter::GetBitmap(const BitmapElement& item)
		{
			const std::u16string& name = item.Name();

			if (m_cached_images.count(name))
			{
				return m_cached_images[name];
			}

			item.ExportImage(sprite_temp_path);

			wk::RawImageRef image;
			wk::InputFileStream file(sprite_temp_path);
			wk::stb::load_image(file, image);
			m_cached_images[name] = image;

			return image;
		}

		void SCWriter::AddGraphicGroup(const GraphicGroup& group)
		{
			m_graphic_groups.push_back(group);
		}
	}
}