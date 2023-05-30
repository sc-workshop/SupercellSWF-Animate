#include "Publisher/SWF/Writer.h"

namespace sc {
	namespace Adobe {
		Result Writer::Init(PIFCMCallback callback, const PublisherConfig& config) {
			if (!callback) {
				return FCM_EXPORT_FAILED;
			}
			m_callback = callback;
			m_config = config;

			console.Init("Writer", m_callback);

			return FCM_SUCCESS;
		}

		pSharedMovieclipWriter Writer::AddMovieclip() {
			MovieclipWriter* builder = new MovieclipWriter();
			builder->Init(this, m_callback);

			return pSharedMovieclipWriter(builder);
		}

		pSharedShapeWriter Writer::AddShape() {
			ShapeWriter* builder = new ShapeWriter();
			builder->Init(this, m_callback);

			return pSharedShapeWriter(builder);
		}

		void Writer::FinalizeAtlas() {
			std::vector<AtlasGeneratorItem> items;

			for (Sprite& sprite : sprites) {
				items.push_back({ sprite.image });
			}

			AtlasGeneratorConfig config;
			vector<cv::Mat> textures;
			AtlasGenerator::Generate(items, textures, config);

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

		void Writer::Finalize() {
			FinalizeAtlas();

			swf.useExternalTexture(m_config.hasTexture);
			swf.save(m_config.output, m_config.compression);
		}
	}
}