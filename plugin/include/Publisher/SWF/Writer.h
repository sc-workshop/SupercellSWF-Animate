#pragma once

#include "Publisher/Shared/SharedWriter.h"

#include "Publisher/SWF/MovieclipWriter.h"
#include "Publisher/SWF/ShapeWriter.h"

#include "io/Console.h"

#include "SupercellFlash.h"
#include "AtlasGenerator.h"
#include "opencv2/opencv.hpp"

#include <locale> 
#include <codecvt> 

using namespace FCM;

namespace sc {
	namespace Adobe {
		// Helper class for atlas generator
		struct Sprite {
			cv::Mat image;
			DOM::Utils::MATRIX2D matrix;
		};

		class Writer : public SharedWriter {
			Console console;
			PIFCMCallback m_callback = nullptr;
			PublisherConfig m_config;

		public:

			std::vector<Sprite> sprites;

			SupercellSWF swf;

			Result Init(PIFCMCallback callback, const PublisherConfig& config) {
				if (!callback) {
					return FCM_EXPORT_FAILED;
				}
				m_callback = callback;
				m_config = config;

				console.Init("Writer", m_callback);

				return FCM_SUCCESS;
			}

			SharedMovieclipWriter* AddMovieclip() {
				MovieclipWriter* builder = new MovieclipWriter();
				builder->Init(this, m_callback);

				return builder;
			}

			SharedShapeWriter* AddShape() {
				ShapeWriter* builder = new ShapeWriter();
				builder->Init(this, m_callback);

				return builder;
			}

			void FinalizeAtlas() {
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

						command->textureIndex(item.textureIndex);

						for (auto point : item.polygon) {
							ShapeDrawBitmapCommandVertex* vertex = new ShapeDrawBitmapCommandVertex();

							vertex->u(point.uv.first / (float)textures[item.textureIndex].cols);
							vertex->v(point.uv.second / (float)textures[item.textureIndex].rows);

							vertex->x(
								point.xy.first
							);
							vertex->y(
								point.xy.second
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

			void Finalize() {
				FinalizeAtlas();

				swf.useExternalTexture(m_config.hasTexture);
				swf.save(m_config.output, m_config.compression);

				delete this;
			}
		};
	}
}