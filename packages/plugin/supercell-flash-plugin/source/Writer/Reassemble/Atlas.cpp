#include "Atlas.h"

namespace sc::flash
{
	void get_sprite(RawImageRef& texture, AtlasGenerator::RectUV bound, const std::vector<AtlasGenerator::PointUV>& points, RawImageRef& result)
	{
		uint16_t width = (uint16_t)std::max<uint32_t>(std::abs(bound.left - bound.right), 1);
		uint16_t height = (uint16_t)std::max<uint32_t>(std::abs(bound.bottom - bound.top), 1);

		AtlasGenerator::PointUV offset{ bound.left, bound.bottom };

		Image::PixelDepth depth = Image::PixelDepth::RGBA8;
		switch (texture->base_type())
		{
		case Image::BasePixelType::RGBA:
			depth = Image::PixelDepth::RGBA8;
			break;
		case Image::BasePixelType::RGB:
			depth = Image::PixelDepth::RGB8;
			break;
		case Image::BasePixelType::LA:
			depth = Image::PixelDepth::LUMINANCE8_ALPHA8;
			break;
		case Image::BasePixelType::L:
			depth = Image::PixelDepth::LUMINANCE8;
			break;
		default:
			break;
		}

		result = CreateRef<RawImage>(width, height, depth, Image::ColorSpace::Linear);

		std::vector<AtlasGenerator::PointUV> polygon(points.size());
		std::transform(points.begin(), points.end(), polygon.begin(), [&offset](const AtlasGenerator::PointUV& point)
			{
				return AtlasGenerator::PointUV{ (uint16_t)(point.x - offset.x), (uint16_t)(point.y - offset.y) };
			}
		);

		for (uint16_t w = 0; width > w; w++) {
			for (uint16_t h = 0; height > h; h++) {
				if (Geometry::point_inside_polygon(polygon, { w, h }) || width == 1 || height == 1)
				{
					uint8_t* source = texture->at(w + offset.x, h + offset.y);
					uint8_t* destination = result->at(w, h);

					Memory::copy(source, destination, Image::PixelDepthTable[(uint16_t)depth].byte_count);
				}
			}
		}
	}

	bool is_solid_9slice(const Shape& shape)
	{
		for (uint32_t i = 0; shape.commands.size() > i; i++)
		{
			uint8_t neighbors_count = 0;
			for (uint32_t t = 0; shape.commands.size() > t; t++)
			{
				if (i == t) continue;

				const ShapeDrawBitmapCommand& first = shape.commands[i];
				const ShapeDrawBitmapCommand& second = shape.commands[t];
				bool valid = false;

				if (shape.commands[i].texture_index != shape.commands[t].texture_index) return false;
				if (first.vertices.size() != second.vertices.size()) return false;

				for (uint32_t v = 0; first.vertices.size() > v; v++)
				{
					const ShapeDrawBitmapCommandVertex& vertex = first.vertices[v];
					auto it = std::find_if(
						second.vertices.begin(), second.vertices.end(),
						[&vertex](const auto& other)
						{
							return other.u == vertex.u && other.v == vertex.v;
						}
					);

					valid = it != second.vertices.end();

					if (valid) break;
				}

				if (valid) neighbors_count++;
			}

			// command should has at least 2 neighbor command to be valid
			// TODO: make it possible to split such neighbors to groups
			if (2 > neighbors_count)
			{
				return false;
			}
		}

		return true;
	}

	ShapeDrawBitmapCommand create_proxy_9slice_command(const Shape& shape)
	{
		ShapeDrawBitmapCommand result;
		result.texture_index = shape.commands[0].texture_index;

		ShapeDrawBitmapCommandVertex min{ cord_max, cord_max, cord_max, cord_max };
		ShapeDrawBitmapCommandVertex max{ cord_min, cord_min, cord_min, cord_min };

		for (const ShapeDrawBitmapCommand& command : shape.commands)
		{
			for (const ShapeDrawBitmapCommandVertex& vertex : command.vertices)
			{
				min.u = std::min(vertex.u, min.u);
				min.v = std::min(vertex.v, min.v);
				min.x = std::min(vertex.x, min.x);
				min.y = std::min(vertex.y, min.y);

				max.u = std::max(vertex.u, max.u);
				max.v = std::max(vertex.v, max.v);
				max.x = std::max(vertex.x, max.x);
				max.y = std::max(vertex.y, max.y);
			}
		}

		result.vertices.emplace_back(ShapeDrawBitmapCommandVertex{
			min.x, max.y,
			min.u, max.v
			});

		result.vertices.emplace_back(ShapeDrawBitmapCommandVertex{
			max.x, max.y,
			max.u, max.v
			});

		result.vertices.emplace_back(ShapeDrawBitmapCommandVertex{
			max.x, min.y,
			max.u, min.v
			});

		result.vertices.emplace_back(ShapeDrawBitmapCommandVertex{
			min.x, min.y,
			min.u, min.v
			});

		return result;
	}

	void repack_atlas(SupercellSWF& swf)
	{
		// Id of shapes for nine scaling
		std::set<uint16_t> nine_scalings_shapes;

		for (MovieClip& movieclip : swf.movieclips)
		{
			if (!movieclip.scaling_grid.has_value()) continue;

			for (DisplayObjectInstance& children : movieclip.childrens)
			{
				DisplayObject& object = swf.GetDisplayObjectByID(children.id);

				if (object.is_shape() && is_solid_9slice((Shape&)object))
				{
					nine_scalings_shapes.insert(object.id);
				}
			}
		}

		// sorted bitmaps to be packed
		std::vector<ShapeDrawBitmapCommand> bitmaps;
		bitmaps.reserve((size_t)swf.shapes.size() * 2);

		// index to bitmap for each bitmap in swf
		std::vector<size_t> indices;
		indices.reserve((size_t)swf.shapes.size() * 4);

		auto get_index = [&bitmaps](const ShapeDrawBitmapCommand& bitmap) -> std::optional<size_t>
			{
				for (size_t i = 0; bitmaps.size() > i; i++)
				{
					const ShapeDrawBitmapCommand& other = bitmaps[i];

					if (bitmap.texture_index != other.texture_index) continue;
					if (bitmap.vertices.size() != other.vertices.size()) continue;

					for (uint32_t t = 0; bitmap.vertices.size() > t; t++)
					{
						const ShapeDrawBitmapCommandVertex& v1 = bitmap.vertices[t];
						const ShapeDrawBitmapCommandVertex& v2 = other.vertices[t];

						if (v1.u != v2.u || v1.v != v2.v)
						{
							goto NEXT_BITMAP;
						}
					}

					return i;

				NEXT_BITMAP:
					continue;
				}

				return std::nullopt;
			};

		// Sorting all shapes
		for (Shape& shape : swf.shapes)
		{
			auto push_bitmap = [&](ShapeDrawBitmapCommand& bitmap)
				{
					auto index = get_index(bitmap);

					if (index.has_value())
					{
						indices.push_back(index.value());
					}
					else
					{
						indices.push_back(bitmaps.size());
						bitmaps.push_back(bitmap);
					}
				};

			if (nine_scalings_shapes.count(shape.id))
			{
				ShapeDrawBitmapCommand proxy = create_proxy_9slice_command(shape);
				push_bitmap(proxy);

				// Denormalize 9slice commands right there because we need original coords in future
				for (ShapeDrawBitmapCommand& bitmap : shape.commands)
				{
					auto& texture = swf.textures[bitmap.texture_index];
					for (auto& vertex : bitmap.vertices)
					{
						vertex.u = (uint16_t)std::ceil(vertex.u * texture.image()->width());
						vertex.v = (uint16_t)std::ceil(vertex.v * texture.image()->height());
					}
				}
			}
			else
			{
				for (ShapeDrawBitmapCommand& bitmap : shape.commands)
				{
					push_bitmap(bitmap);
				}
			};
		}

		// Decompressing all images to RawImage and converting to raw pixel format
		std::vector<Ref<RawImage>> images;
		images.resize(swf.textures.size());

		parallel::enumerate(swf.textures.begin(), swf.textures.end(), [&images](const SWFTexture& texture, size_t n)
			{
				Ref<RawImage> image = texture.raw_image();
				Ref<RawImage> result = image;

				switch (image->base_type())
				{
				case Image::BasePixelType::RGBA:
					if (image->depth() != Image::PixelDepth::RGBA8)
					{
						result = CreateRef<RawImage>(image->width(), image->height(), Image::PixelDepth::RGBA8);
						RawImage::remap(image->data(), result->data(), image->width(), image->height(), image->depth(), result->depth());
					}
					break;
				case Image::BasePixelType::RGB:
					if (image->depth() != Image::PixelDepth::RGB8)
					{
						result = CreateRef<RawImage>(image->width(), image->height(), Image::PixelDepth::RGB8);
						RawImage::remap(image->data(), result->data(), image->width(), image->height(), image->depth(), result->depth());
					}
					break;
				case Image::BasePixelType::LA:
					if (image->depth() != Image::PixelDepth::LUMINANCE8_ALPHA8)
					{
						result = CreateRef<RawImage>(image->width(), image->height(), Image::PixelDepth::LUMINANCE8_ALPHA8);
						RawImage::remap(image->data(), result->data(), image->width(), image->height(), image->depth(), result->depth());
					}
					break;
				case Image::BasePixelType::L:
					if (image->depth() != Image::PixelDepth::LUMINANCE8)
					{
						result = CreateRef<RawImage>(image->width(), image->height(), Image::PixelDepth::LUMINANCE8);
						RawImage::remap(image->data(), result->data(), image->width(), image->height(), image->depth(), result->depth());
					}
					break;
				default:
					break;
				}

				images[n] = result;
			});

		// Cutting sprites from atlases
		std::vector<Ref<AtlasGenerator::Item>> items;
		items.resize(bitmaps.size());

		std::launch policy = std::launch::deferred;
#if !WK_DEBUG
		policy |= std::launch::async;
#endif // !WK_DEBUG


		parallel::enumerate(bitmaps.begin(), bitmaps.end(), [&images, &items](const ShapeDrawBitmapCommand& bitmap, size_t n)
			{
				auto& texture = images[bitmap.texture_index];
				AtlasGenerator::RectUV bitmap_bound
				{
					0xFFFF, 0,
					0, 0xFFFF
				};

				std::vector<AtlasGenerator::PointUV> points;
				points.reserve(bitmap.vertices.size());

				for (const ShapeDrawBitmapCommandVertex& vertex : bitmap.vertices)
				{
					AtlasGenerator::PointUV point(
						(uint16_t)std::ceil(vertex.u * texture->width()),
						(uint16_t)std::ceil(vertex.v * texture->height())
					);
					points.push_back(point);

					bitmap_bound.left = std::min(point.x, bitmap_bound.left);
					bitmap_bound.bottom = std::min(point.y, bitmap_bound.bottom);
					bitmap_bound.right = std::max(point.x, bitmap_bound.right);
					bitmap_bound.top = std::max(point.y, bitmap_bound.top);
				}

				uint16_t width = (uint16_t)std::abs(bitmap_bound.left - bitmap_bound.right);
				uint16_t height = (uint16_t)std::abs(bitmap_bound.bottom - bitmap_bound.top);
				bool zero_width = 1 >= width;
				bool zero_height = 1 >= height;
				bool colorfill = (zero_width && zero_height);
				bool dumb_vertices = false; // dark magic. dont touch.
				if (!colorfill)
				{
					std::set<std::tuple<float, float>> vertices_set;
					for (auto& vertex : bitmap.vertices)
					{
						if (vertices_set.size() >= 3) break;
						vertices_set.insert(std::make_tuple(vertex.u, vertex.v));
					}

					std::vector<std::tuple<float, float>> vertices;
					vertices.assign(vertices_set.begin(), vertices_set.end());
					if (vertices_set.size() == 2)
					{
						auto& p1 = vertices[0];
						auto& p2 = vertices[1];
						auto& [x1, y1] = p1;
						auto& [x2, y2] = p2;
						if (x1 != x2 && y1 != y2)
						{
							dumb_vertices = true;
						}
					}
				}

				auto process_colorfill = [&]()
					{
						ColorRGBA color(0, 0, 0, 0xFF);

						uint16_t x = bitmap_bound.left;
						uint16_t y = bitmap_bound.bottom;
						switch (texture->base_type())
						{
						case Image::BasePixelType::RGBA:
							color = texture->at<ColorRGBA>(x, y);
							break;
						case Image::BasePixelType::RGB:
						{
							ColorRGB& pixel = texture->at<ColorRGB>(x, y);
							color.r = pixel.r;
							color.g = pixel.g;
							color.b = pixel.b;
						}
						break;
						case Image::BasePixelType::LA:
						{
							ColorLA& pixel = texture->at<ColorLA>(x, y);
							color.r = pixel.l;
							color.g = pixel.l;
							color.b = pixel.l;
							color.a = pixel.a;
						}
						break;
						case Image::BasePixelType::L:
						{
							uint8_t& pixel = texture->at<uint8_t>(x, y);
							color.r = pixel;
							color.g = pixel;
							color.b = pixel;
						}
						break;
						default:
							throw wk::Exception();
						}

						items[n] = CreateRef<AtlasGenerator::Item>(color);
						items[n]->mark_as_preprocessed();
					};

				auto process_sprite = [&]()
					{
						RawImageRef sprite;
						get_sprite(texture, bitmap_bound, points, sprite);

						std::set<std::tuple<float, float>> points_set;
						auto item = CreateRef<AtlasGenerator::Item>(*sprite);
						items[n] = item;

						item->vertices.reserve(bitmap.vertices.size());
						for (AtlasGenerator::PointUV& point : points)
						{
							auto& item_vertex = item->vertices.emplace_back();

							item_vertex.uv.x = (point.x - bitmap_bound.left);
							item_vertex.uv.y = (point.y - bitmap_bound.bottom);

							if (zero_width || zero_height)
							{
								if (points_set.count({ point.x, point.y }))
								{
									item_vertex.uv.x += zero_width ? 1 : 0;
									item_vertex.uv.y += zero_height ? 1 : 0;
								}
								else
								{
									points_set.insert({ point.x, point.y });
								}
							}
						}

						bool success = item->mark_as_custom();
						if (!success)
						{
							throw Exception("Looks like this sc contains non-convex polygons and this is really bad! Atlas repackaging is impossible....");
						}
					};

				if (colorfill || dumb_vertices)
				{
					process_colorfill();
				}
				else
				{
					process_sprite();
				}

			}, policy
		);

		Adobe::SCPlugin& context = Adobe::SCPlugin::Instance();
		auto* status = context.Window()->CreateStatusBarComponent(
			context.locale.GetString("TID_STATUS_SPRITE_PACK")
		);

		AtlasGenerator::Config config(4096, 4096, 1.0f, 2);

		int itemCount = (int)items.size();
		status->SetRange(itemCount);

		config.progress = [&status](uint32_t value) {
			status->SetProgress(value);
			};

		AtlasGenerator::Generator generator(config);
		size_t atlas_count = 0;
		try {
			using AtlasInput = std::reference_wrapper<AtlasGenerator::Item>;
			AtlasGenerator::Container<AtlasInput> input;
			input.reserve(items.size());
			for (auto& item : items)
			{
				input.emplace_back(*item.get());
			}

			atlas_count = generator.generate<AtlasInput>(input);
			context.Window()->DestroyStatusBar(status);
		}
		catch (const AtlasGenerator::PackagingException& exception)
		{
			throw Exception(exception.what());
		}

		if (atlas_count >= std::numeric_limits<uint16_t>().max())
		{
			throw Exception("Failed to repack. Too many textures!");
		}

		swf.textures.clear();

		for (size_t i = 0; atlas_count > i; i++)
		{
			RawImage& atlas = generator.get_atlas(i);

			auto& texture = swf.textures.emplace_back();
			texture.load_from_image(atlas);
		}

		size_t bitmap_counter = 0;
		for (Shape& shape : swf.shapes)
		{
			if (nine_scalings_shapes.count(shape.id))
			{
				size_t bitmap_index = bitmap_counter++;
				AtlasGenerator::Item& item = *items[indices[bitmap_index]];
				auto& texture = swf.textures[item.texture_index];

				wk::PointF uv_offset{ cord_max, cord_max };

				for (ShapeDrawBitmapCommand& command : shape.commands)
				{
					for (auto& vertex : command.vertices)
					{
						uv_offset.x = std::min(vertex.u, uv_offset.x);
						uv_offset.y = std::min(vertex.v, uv_offset.y);
					}
				}

				for (ShapeDrawBitmapCommand& command : shape.commands)
				{
					command.texture_index = (uint16_t)item.texture_index;
					for (auto& vertex : command.vertices)
					{
						wk::PointF uv_vertex{
							vertex.u - uv_offset.x,
							vertex.v - uv_offset.y
						};

						item.transform.transform_point(uv_vertex);

						vertex.u = uv_vertex.x / texture.image()->width();
						vertex.v = uv_vertex.y / texture.image()->height();
					}
				}

				continue;
			}

			for (ShapeDrawBitmapCommand& command : shape.commands)
			{
				AtlasGenerator::Item& item = *items[indices[bitmap_counter++]];
				command.texture_index = (uint8_t)item.texture_index;
				auto& texture = swf.textures[command.texture_index];

				auto colorfill = item.get_colorfill();

				if (colorfill.has_value())
				{
					AtlasGenerator::Vertex vertex = colorfill.value();
					item.vertices.clear();
					item.vertices = AtlasGenerator::Container<AtlasGenerator::Vertex>(command.vertices.size(), vertex);
				}

				for (uint32_t i = 0; command.vertices.size() > i; i++)
				{
					auto& orig_vertex = command.vertices[i];
					auto new_vertex = item.vertices[i];
					item.transform.transform_point(new_vertex.uv);

					orig_vertex.u = (float)new_vertex.uv.x / (float)texture.image()->width();
					orig_vertex.v = (float)new_vertex.uv.y / (float)texture.image()->height();
				}
			}
		}
	}

	void remove_unused_textures(SupercellSWF& swf)
	{
		std::map<uint16_t, SWFTexture> used_textures;

		for (const auto& shape : swf.shapes)
		{
			for (const auto& command : shape.commands)
			{
				used_textures[command.texture_index] = swf.textures[command.texture_index];
			}
		}

		uint16_t index = 0;
		SupercellSWF::TextureArray new_textures;
		new_textures.reserve(used_textures.size());

		std::unordered_map<uint16_t, uint16_t> texture_indices_mapping;
		for (std::map<uint16_t, SWFTexture>::iterator it = used_textures.begin(); it != used_textures.end(); ++it)
		{
			new_textures.push_back(it->second);
			texture_indices_mapping[it->first] = index++;
		}

		swf.textures = new_textures;
		for (auto& shape : swf.shapes)
		{
			for (auto& command : shape.commands)
			{
				command.texture_index = texture_indices_mapping[command.texture_index];
			}
		}
	}
}