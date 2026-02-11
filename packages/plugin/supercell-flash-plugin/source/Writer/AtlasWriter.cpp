#include "AtlasWriter.h"
#include "Writer.h"
#include "Module/SCPluginException.h"
#include "core/parallel/enumerate.h"

#include "atlas_generator/Generator.h"
#include "atlas_generator/PackagingException.h"

#include "Reassemble/Atlas.h"

namespace sc::Adobe {
	using namespace Animate::Publisher;

	AtlasWriter::AtlasWriter(SCWriter& writer) : m_writer(writer)
	{
	}

	void AtlasWriter::ProcessDrawCommand(
		flash::ShapeDrawBitmapCommand& command,
		wk::AtlasGenerator::Item::Transformation& transform,
		GraphicItem& item
	)
	{
		using namespace wk;
		using namespace wk::AtlasGenerator;

		wk::Matrix2D matrix = item.Transformation2D();
		flash::SWFTexture& texture = m_writer.swf.textures[command.texture_index];
		if (command.vertices.empty()) return;

		// Copy the last vertex until size equals 4, this is important
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

	void AtlasWriter::ProcessSpriteItem(
		flash::Shape& shape,
		wk::AtlasGenerator::Item& atlas_item,
		BitmapItem& sprite_item
	)
	{
		ProcessVertices(shape, atlas_item.vertices, atlas_item, sprite_item);
	}

	void AtlasWriter::ProcessSlicedItem(
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

		Container<Container<VertexF>> regions;
		atlas_item.get_9slice(
			sliced_item.Guides(),
			regions, transform
		);

		for (const Container<VertexF>& region : regions)
		{
			ProcessVertices(shape, region, atlas_item, sliced_item);
		}
	}

	void AtlasWriter::WriteAtlas()
	{
		using namespace wk;

		SCConfig& config = SCPlugin::Publisher::ActiveConfig();
		SCPlugin& context = SCPlugin::Instance();

		StatusComponent* status = context.Window()->CreateStatusBarComponent(
			context.locale.GetString("TID_STATUS_SPRITE_PACK")
		);

		std::vector<AtlasGenerator::Item> items;

		for (StaticElementsGroup& group : m_writer.m_graphic_groups)
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
				for (size_t group_index = 0, atlas_item_index = 0; m_writer.m_graphic_groups.size() >= group_index; group_index++)
				{
					StaticElementsGroup& group = m_writer.m_graphic_groups[group_index];
					for (size_t group_item_index = 0; group.Size() > group_item_index; group_item_index++)
					{
						if (atlas_item_index == exception.index())
						{
							symbol_name = m_writer.m_graphic_groups[group_index][group_item_index].Symbol().name;
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

			flash::SWFTexture& texture = m_writer.swf.textures.emplace_back();
			texture.load_from_image(atlas);
		}

		uint16_t command_index = 0;
		uint16_t shape_index = (m_writer.swf.shapes.size() - m_writer.m_graphic_groups.size());
		for (uint32_t group_index = 0; m_writer.m_graphic_groups.size() > group_index; group_index++, shape_index++)
		{
			flash::Shape& shape = m_writer.swf.shapes[shape_index];
			StaticElementsGroup& group = m_writer.m_graphic_groups[group_index];

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
			flash::repack_atlas(m_writer.swf);
		}
		else
		{
			flash::remove_unused_textures(m_writer.swf);
		}

		wk::parallel::enumerate(
			m_writer.swf.textures.begin(),
			m_writer.swf.textures.end(),
			[&config](flash::SWFTexture& texture, size_t)
			{
				using flash::SWFTexture;
				wk::Ref<wk::RawImage> atlas;

				bool generateStreaming = config.textureEncoding == SWFTexture::TextureEncoding::SupercellTexture &&
					config.generateStreamingTexture;

				if (generateStreaming)
					atlas = texture.raw_image();

				texture.encoding(config.textureEncoding);
				if (config.textureEncoding == SWFTexture::TextureEncoding::Raw)
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

				if (generateStreaming) {
					wk::Ref<texture::SupercellTexture> image = std::static_pointer_cast<texture::SupercellTexture>(texture.image());

					// No idea how does this work
					// But we simply repeat configuration of most of sctx files
					image->streaming_ids = { 3 };
					image->streaming_variants = std::vector<texture::SupercellTexture>();
					wk::RawImage streamingTexture(
						(uint16_t)floor<uint16_t>(atlas->width() / 8),
						(uint16_t)floor<uint16_t>(atlas->height() / 8),
						atlas->depth(),
						atlas->colorspace()
					);

					atlas->copy(streamingTexture);
					image->streaming_variants->emplace_back(streamingTexture, image->pixel_type());
				}
			}
		);

		if (config.type == SCConfig::SWFType::SC2)
		{
			for (auto& shape : m_writer.swf.shapes)
			{
				for (flash::ShapeDrawBitmapCommand& command : shape.commands)
				{
					command.sort_advanced_vertices(true);
				}
			}
		}
	}

	void AtlasWriter::ProcessFilledItem(
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
			shape_command.texture_index = (uint32_t)atlas_item.texture_index + texture_offset;

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
}
