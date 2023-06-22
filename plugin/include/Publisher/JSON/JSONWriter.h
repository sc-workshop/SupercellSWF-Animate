#pragma once

#include "Publisher/Shared/SharedWriter.h"

#include "JSONMovieclipWriter.h"
#include "JSONShapeWriter.h"

#include "JSON.hpp"

using namespace nlohmann;

namespace sc {
	namespace Adobe {
		class JSONWriter : public SharedWriter {
			json m_shapes = json::array();
			json m_textfields = json::array();
			json m_modifiers = json::array();
			json m_movieclips = json::array();

		public:
			using SharedWriter::SharedWriter;

			fs::path imageFolder;
			uint32_t imageCount = 0;

			void Init() {
				imageFolder = m_context.config.output.parent_path() / "images";
				if (fs::exists(imageFolder)) {
					fs::remove_all(imageFolder);
				}
				fs::create_directory(imageFolder);
			}

			pSharedMovieclipWriter AddMovieclip() {
				JSONMovieclipWriter* builder = new JSONMovieclipWriter();
				builder->Init(this);

				return pSharedMovieclipWriter(builder);
			}

			void AddMovieclip(json movieclip) {
				m_movieclips.push_back(movieclip);
			}

			pSharedShapeWriter AddShape() {
				JSONShapeWriter* builder = new JSONShapeWriter();
				builder->Init(this);

				return pSharedShapeWriter(builder);
			}

			void AddShape(json shape) {
				m_shapes.push_back(shape);
			}

			void AddModifier(uint16_t id, sc::MovieClipModifier::Type type) {
				m_modifiers.push_back({
					{"id", id},
					{"type", (uint8_t)type}
				});
			}

			void AddTextField(uint16_t id, TextFieldInfo field) {
				std::string alignMode;
				switch (field.style.alignment)
				{
				case DOM::FrameElement::AlignMode::ALIGN_MODE_CENTER:
					alignMode = "center";
					break;
				case DOM::FrameElement::AlignMode::ALIGN_MODE_JUSTIFY:
					alignMode = "justify";
					break;
				case DOM::FrameElement::AlignMode::ALIGN_MODE_LEFT:
					alignMode = "left";
					break;
				case DOM::FrameElement::AlignMode::ALIGN_MODE_RIGHT:
					alignMode = "right";
					break;
				default:
					break;
				}


				m_textfields.push_back(json::object({
					{"id", id},
					{"bound", json::array({
						field.bound.bottomRight.x,
						field.bound.bottomRight.y,
						field.bound.topLeft.x,
						field.bound.topLeft.y
					})},
					{"text", Utils::ToUtf8(field.text)},
					{"outline", field.isOutlined},
					{"outlineColor", Utils::ToString(field.outlineColor)},
					{"align", alignMode},
					{"indent", field.style.indent},
					{"space", field.style.lineSpacing},
					{"leftMargin", field.style.leftMargin},
					{"rightMargin", field.style.rightMargin},
					{"fontColor", Utils::ToString(field.fontColor)},
					{"fontSize", field.fontSize},
					{"fontStyle", field.fontStyle},
					{"fontName", Utils::ToUtf8(field.fontName)},
					{"autoKern", field.autoKern},
				}));
			}

			void Finalize() {
				json root = {
					{"shapes", m_shapes },
					{"textfields", m_textfields },
					{"modifiers", m_modifiers},
					{"movieclips", m_movieclips}
				};

				std::ofstream file(m_context.config.output);
				file << setw(4) << root << endl;
				file.close();
			}
		};
	}
}