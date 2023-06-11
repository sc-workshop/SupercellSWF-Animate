#include "Publisher/SWF/Writer.h"

namespace sc {
	namespace Adobe {
		pSharedMovieclipWriter Writer::AddMovieclip() {
			MovieclipWriter* builder = new MovieclipWriter();
			builder->Init(this);

			return pSharedMovieclipWriter(builder);
		}

		pSharedShapeWriter Writer::AddShape() {
			ShapeWriter* builder = new ShapeWriter();
			builder->Init(this);

			return pSharedShapeWriter(builder);
		}

		void Writer::AddModifier(uint16_t id, sc::MovieClipModifier::Type type) {
			pMovieClipModifier modifier = pMovieClipModifier(new MovieClipModifier());
			modifier->id(id);
			modifier->type(type);
			swf.movieClipModifiers.push_back(modifier);
		}

		void Writer::Finalize() {
			PublisherConfig& config = m_context.config;

			// Atlas processing
			{
				std::vector<AtlasGeneratorItem> items;

				for (Sprite& sprite : sprites) {
					items.push_back({ sprite.image });
				}

				AtlasGeneratorConfig config;
				vector<cv::Mat> textures;
				AtlasGeneratorResult packageResult = AtlasGenerator::Generate(items, textures, config);
				switch (packageResult) {
				case AtlasGeneratorResult::BAD_POLYGON:
					throw exception("[AtlasGenerator] Failed to generate polygon");
				case AtlasGeneratorResult::TOO_MANY_IMAGES:
					throw exception("[AtlasGenerator] Too many image for one sheet");
				case AtlasGeneratorResult::OK:
					break;
				default:
					throw exception("[AtlasGenerator] Unknown error");
				}

				uint16_t itemIndex = 0;
				for (pShape& shape : swf.shapes) {
					for (uint16_t i = 0; shape->commands.size() > i; i++) {
						ShapeDrawBitmapCommand* command = new ShapeDrawBitmapCommand();
						AtlasGeneratorItem& item = items[itemIndex];
						DOM::Utils::MATRIX2D& matrix = sprites[itemIndex].matrix;

						command->textureIndex(item.textureIndex);

						for (auto point : item.polygon) {
							ShapeDrawBitmapCommandVertex* vertex = new ShapeDrawBitmapCommandVertex();

							vertex->u(point.uv.first / (float)textures[item.textureIndex].cols);
							vertex->v(point.uv.second / (float)textures[item.textureIndex].rows);

							vertex->x(
								point.xy.first * matrix.a + point.xy.second * matrix.b + matrix.tx
							);
							vertex->y(
								point.xy.first * matrix.c + point.xy.second * matrix.d + matrix.ty
							);

							command->vertices.push_back(pShapeDrawBitmapCommandVertex(vertex));
						}

						itemIndex++;
						shape->commands[i] = pShapeDrawBitmapCommand(command);
					}
				}

				for (cv::Mat& atlas : textures) {
					SWFTexture* texture = new SWFTexture();

					texture->width(atlas.cols);
					texture->height(atlas.rows);

					switch (config.textureType) {
					case AtlasGeneratorConfig::TextureType::RGBA:
						// TODO: Texture quality property
						cv::cvtColor(atlas, atlas, cv::COLOR_BGRA2RGBA);
						break;
					default:
						break;
					}

					texture->data = std::vector<uint8_t>(atlas.datastart, atlas.dataend);

					swf.textures.push_back(pSWFTexture(texture));
				}
			}

			swf.useExternalTexture(config.hasTexture);
			swf.save(config.output, config.compression);
		}
	}
}