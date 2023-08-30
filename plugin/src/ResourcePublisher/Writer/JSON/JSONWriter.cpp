#include "ResourcePublisher/Writer/JSON/JSONWriter.h"
#include "Module/Context.h"

namespace sc {
	namespace Adobe {
		void JSONWriter::Init(Context& context) {
			m_context = &context;

			saveName = m_context->config.output.stem().u16string();
			saveFolder = m_context->config.output.parent_path();

			imageFolder = fs::path(saveFolder / "images");
			if (fs::exists(imageFolder)) {
				fs::remove_all(imageFolder);
			}

			fs::create_directory(imageFolder);
		}

		pSharedMovieclipWriter JSONWriter::AddMovieclip() {
			return pSharedMovieclipWriter(new JSONMovieclipWriter(this));
		}

		void JSONWriter::AddMovieclip(ordered_json& movieclip) {
			m_movieclips.push_back(movieclip);
		}

		pSharedShapeWriter JSONWriter::AddShape() {
			return pSharedShapeWriter(new JSONShapeWriter(this));
		}

		void JSONWriter::AddShape(ordered_json& shape) {
			m_shapes.push_back(shape);
		}

		void JSONWriter::AddModifier(uint16_t id, sc::MovieClipModifier::Type type) {
			m_modifiers.push_back(ordered_json(
				{
					{"id", id},
					{"type", (uint8_t)type}
				}
			));
		}

		void JSONWriter::AddTextField(uint16_t id, TextFieldInfo field) {
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

			m_textfields.push_back(ordered_json(
				{
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
					{"autoKern", field.autoKern}
				}
			));
		}

		void JSONWriter::AddExportName(uint16_t id, std::u16string name) {
			for (auto object : m_movieclips) {
				if (object["id"].is_number() && object["id"] == id) {
					object["exports"].push_back(Utils::ToUtf8(name));
				}
			}
		}

		void JSONWriter::Finalize() {
			ordered_json root = ordered_json(
				{
					{"shapes", m_shapes },
					{"textfields", m_textfields },
					{"modifiers", m_modifiers},
					{"movieclips", m_movieclips}
				}
			);

			std::ofstream file(saveFolder / (saveName + u".json"));
			file << setw(4) << root << endl;
			file.close();
		}
	}
}