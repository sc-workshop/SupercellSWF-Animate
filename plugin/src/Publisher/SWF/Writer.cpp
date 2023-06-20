#include "Publisher/SWF/Writer.h"

#include "DOM/FrameElement/ITextStyle.h"

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

		void Writer::AddTextField(uint16_t id, TextFieldInfo field) {
			pTextField textfield = pTextField(new TextField());

			textfield->id(id);

			textfield->text(Utils::ToUtf8(field.text));

			textfield->fontName(Utils::ToUtf8(field.fontName));
			textfield->fontColor(*(uint32_t*)&field.fontColor);
			textfield->fontSize(
				(uint8_t)clamp(field.fontSize, (uint16_t)0, (uint16_t)UINT8_MAX)
			);

			switch (field.style.alignment)
			{
			case DOM::FrameElement::AlignMode::ALIGN_MODE_CENTER:
				textfield->fontAlign(sc::TextField::Align::Center);
				break;
			case DOM::FrameElement::AlignMode::ALIGN_MODE_JUSTIFY:
				textfield->fontAlign(sc::TextField::Align::Justify);
				break;
			case DOM::FrameElement::AlignMode::ALIGN_MODE_LEFT:
				textfield->fontAlign(sc::TextField::Align::Left);
				break;
			case DOM::FrameElement::AlignMode::ALIGN_MODE_RIGHT:
				textfield->fontAlign(sc::TextField::Align::Right);
				break;
			default:
				break;
			}

			textfield->left((int16_t)ceil(field.bound.topLeft.x));
			textfield->top((int16_t)ceil(field.bound.topLeft.y));

			textfield->right((int16_t)ceil(field.bound.bottomRight.x));
			textfield->bottom((int16_t)ceil(field.bound.bottomRight.y));

			if (field.fontStyle != DOM::FrameElement::REGULAR_STYLE_STR) {
				if (field.fontStyle != DOM::FrameElement::ITALIC_STYLE_STR) {
					textfield->isItalic(true);
				}
				else if (field.fontStyle != DOM::FrameElement::BOLD_STYLE_STR) {
					textfield->isBold(true);
				}
				else if (field.fontStyle != DOM::FrameElement::BOLD_ITALIC_STYLE_STR) {
					textfield->isBold(true);
					textfield->isItalic(true);
				}
			}

			if (field.renderingMode.aaMode == DOM::FrameElement::AAMode::ANTI_ALIAS_MODE_DEVICE) {
				textfield->useDeviceFont(true);
			}

			textfield->isOutlined(field.isOutlined);
			textfield->outlineColor(*(uint32_t*)&field.outlineColor);
			textfield->autoKern(field.autoKern);
			textfield->isMultiline(
				field.lineMode == DOM::FrameElement::LineMode::LINE_MODE_SINGLE ? false : true
			);
			
			swf.textFields.push_back(textfield);
		}

		void Writer::Finalize() {
			// Atlas processing
			{
				std::vector<AtlasGeneratorItem> items;

				for (Sprite& sprite : sprites) {
					items.push_back({ sprite.image });
				}

				AtlasGeneratorConfig config;
				config.maxSize = { m_context.config.textureMaxWidth, m_context.config.textureMaxHeight };
				config.scaleFactor = m_context.config.textureScaleFactor;

				vector<cv::Mat> textures;
				AtlasGeneratorResult packageResult = AtlasGenerator::Generate(items, textures, config);
				switch (packageResult) {
				case AtlasGeneratorResult::BAD_POLYGON:
					throw exception("[AtlasGenerator] Failed to generate polygon");
				case AtlasGeneratorResult::TOO_MANY_IMAGES:
					throw exception("[AtlasGenerator] Too many image for one sheet");
				case AtlasGeneratorResult::TOO_BIG_IMAGE:
					throw exception("[AtlasGenerator] Some of images are too big. Try increasing texture size to fix error");
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

			swf.useExternalTexture(m_context.config.hasTexture);
			swf.useLowResTexture(false);
			swf.useMultiResTexture(false);
			swf.save(m_context.config.output, m_context.config.compression);
		}
	}
}