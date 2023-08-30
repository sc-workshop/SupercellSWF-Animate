#include "ResourcePublisher/Writer/SWF/Writer.h"
#include "Module/Context.h"

namespace sc {
	namespace Adobe {
		void SCWriter::Init(Context& context) {
			m_context = &context;
		};

		pSharedMovieclipWriter SCWriter::AddMovieclip() {
			return pSharedMovieclipWriter(new SCMovieclipWriter(this));
		}

		pSharedShapeWriter SCWriter::AddShape() {
			return pSharedShapeWriter(new SCShapeWriter(this));
		}

		void SCWriter::AddModifier(uint16_t id, sc::MovieClipModifier::Type type) {
			pMovieClipModifier modifier = pMovieClipModifier(new MovieClipModifier());
			modifier->id(id);
			modifier->type(type);
			swf.movieClipModifiers.push_back(modifier);
		}

		void SCWriter::AddTextField(uint16_t id, TextFieldInfo field) {
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

		void SCWriter::AddExportName(uint16_t id, std::u16string name) {
			pExportName exportName = pExportName(new ExportName());

			exportName->id(id);
			exportName->name(Utils::ToUtf8(name));

			swf.exports.push_back(exportName);
		}

		void SCWriter::LoadExternal() {
			fs::path& filepath = m_context->config.exportToExternalPath;

			if (!fs::exists(filepath)) {
				m_context->trace("External file does not exist");
				return;
			}

			SupercellSWF base_swf;
			base_swf.load(filepath);

			uint16_t idOffset = 0;
			{
				for (pMovieClip object : base_swf.movieClips) {
					if (object->id() > idOffset) {
						idOffset = object->id();
					}
				}

				for (pMovieClipModifier object : base_swf.movieClipModifiers) {
					if (object->id() > idOffset) {
						idOffset = object->id();
					}
				}

				for (pTextField object : base_swf.textFields) {
					if (object->id() > idOffset) {
						idOffset = object->id();
					}
				}

				for (pShape object : base_swf.shapes) {
					if (object->id() > idOffset) {
						idOffset = object->id();
					}
				}
			}
			idOffset++;

			// Display object processing
			for (pMovieClip object : swf.movieClips) {
				object->id(object->id() + idOffset);

				for (pDisplayObjectInstance bind : object->instances) {
					bind->id += idOffset;
				}

				object->matrixBankIndex(object->matrixBankIndex() + (uint8_t)swf.matrixBanks.size());

				base_swf.movieClips.push_back(object);
			}

			for (pMovieClipModifier object : swf.movieClipModifiers) {
				object->id(object->id() + idOffset);

				base_swf.movieClipModifiers.push_back(object);
			}

			for (pTextField object : swf.textFields) {
				object->id(object->id() + idOffset);

				base_swf.textFields.push_back(object);
			}

			for (pShape object : swf.shapes) {
				object->id(object->id() + idOffset);

				for (auto bitmap : object->commands) {
					bitmap->textureIndex(bitmap->textureIndex() + (uint8_t)swf.textures.size());
				}

				base_swf.shapes.push_back(object);
			}

			// Common resources processing
			for (pMatrixBank bank : swf.matrixBanks) {
				base_swf.matrixBanks.push_back(bank);
			}
			for (pSWFTexture texture : swf.textures) {
				base_swf.textures.push_back(texture);
			}
			for (pExportName exportName : swf.exports) {
				exportName->id(exportName->id() + idOffset);
				base_swf.exports.push_back(exportName);
			}

			swf = base_swf;
		}

		void SCWriter::Finalize() {
			if (m_context->progressBar->window->aboutToExit) {
				m_context->close();
				return;
			}

			// Atlas processing
			{
				m_context->progressBar->window->SetProgress((uint8_t)PublisherStage::SpritePackaging);
				m_context->progressBar->window->SetStatus(wxStringU16(m_context->locale.Get("TID_STATUS_TEXTURE_PROCESS")));

				std::vector<AtlasGeneratorItem> items;
				for (Sprite& sprite : sprites) {
					items.push_back({ sprite.image });
				}
				int itemCount = (int)items.size();

				auto width = m_context->config.textureMaxWidth;

				AtlasGeneratorConfig config;
				config.maxSize = { m_context->config.textureMaxWidth, m_context->config.textureMaxHeight };
				config.scaleFactor = m_context->config.textureScaleFactor;

				StatusComponent* textureProgress = m_context->progressBar->window->GetAvailableProgressBar();

				textureProgress->SetLabel(wxStringU16(m_context->locale.Get("TID_BAR_LABEL_TEXURE_PACK")));
				textureProgress->SetStatus(wxStringU16(m_context->locale.Get("TID_STATUS_SPRITE_PACK")));
				textureProgress->SetRange(itemCount);
				config.progress = [textureProgress, itemCount](int value) {
					textureProgress->SetProgress(itemCount - value);
				};

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

				textureProgress->SetLabel(wxStringU16(m_context->locale.Get("TID_STATUS_TEXTURE_SAVE")));

				uint16_t itemIndex = 0;
				uint32_t spriteIndex = 0;
				for (pShape& shape : swf.shapes) {
					for (uint16_t i = 0; shape->commands.size() > i; i++) {
						ShapeDrawBitmapCommand* command = new ShapeDrawBitmapCommand();
						AtlasGeneratorItem& item = items[itemIndex];
						Sprite sprite = sprites[spriteIndex++];
						DOM::Utils::MATRIX2D& matrix = sprite.matrix;

						command->textureIndex(item.textureIndex);

						if (sprite.contour.empty()) {
							for (auto point : item.polygon) {
								ShapeDrawBitmapCommandVertex* vertex = new ShapeDrawBitmapCommandVertex();

								vertex->u(point.uv.first / (float)textures[item.textureIndex].cols);
								vertex->v(point.uv.second / (float)textures[item.textureIndex].rows);

								vertex->x(
									(matrix.a * point.xy.first) + (matrix.c * point.xy.second) + matrix.tx
								);
								vertex->y(
									(matrix.b * point.xy.first) + (matrix.d * point.xy.second) + matrix.ty
								);

								command->vertices.push_back(pShapeDrawBitmapCommandVertex(vertex));
							}
						}
						else {
							float u = item.polygon[0].uv.first / (float)textures[item.textureIndex].cols;
							float v = item.polygon[0].uv.second / (float)textures[item.textureIndex].rows;

							if (sprite.contour.size() < 4) {
								while (sprite.contour.size() != 4) {
									sprite.contour.push_back(sprite.contour[sprite.contour.size() - 1]);
								}
							}

							for (const Point2D& point : sprite.contour) {
								ShapeDrawBitmapCommandVertex* vertex = new ShapeDrawBitmapCommandVertex();

								vertex->u(u);
								vertex->v(v);

								vertex->x(
									(matrix.a * point.x) + (-matrix.b * point.y) + matrix.tx
								);
								vertex->y(
									(-matrix.c * point.x) + (matrix.d * point.y) + matrix.ty
								);

								command->vertices.push_back(pShapeDrawBitmapCommandVertex(vertex));
							}
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

					texture->textureData = std::vector<uint8_t>(atlas.datastart, atlas.dataend);

					if (m_context->config.textureEncoding == SWFTexture::TextureEncoding::Raw) {
						switch (m_context->config.textureQuality)
						{
						case Quality::Highest:
							texture->pixelFormat(SWFTexture::PixelFormat::RGBA8);
							break;
						case Quality::High:
						case Quality::Medium:
							texture->pixelFormat(SWFTexture::PixelFormat::RGBA4);
							break;
						case Quality::Low:
							texture->pixelFormat(SWFTexture::PixelFormat::RGB5_A1);
							break;
						default:
							break;
						}
					}
					else {
						texture->textureEncoding(m_context->config.textureEncoding);
					}

					swf.textures.push_back(pSWFTexture(texture));
				}

				m_context->progressBar->window->DestroyProgressBar(textureProgress);
			}

			if (m_context->config.exportToExternal) {
				LoadExternal();
			}

			if (m_context->progressBar->window->aboutToExit) {
				m_context->close();
				return;
			}
			else {
				m_context->progressBar->window->SetProgress((uint8_t)PublisherStage::Saving);
				m_context->progressBar->window->SetStatus(wxStringU16(m_context->locale.Get("TID_STATUS_SAVE")));

				swf.useExternalTexture(m_context->config.hasExternalTexture);
				swf.useLowResTexture(false);
				swf.useMultiResTexture(false);
				swf.usePrecisionMatrices(m_context->config.hasPrecisionMatrices);

				swf.save(m_context->config.output.replace_extension("sc"), m_context->config.compression);
			}
		}
	}
}