#include "ResourcePublisher/Writer/JSON/JSONWriter.h"
#include "ResourcePublisher/Writer/JSON/JSONMovieclipWriter.h"
#include "Module/Context.h"

namespace sc {
	namespace Adobe {
		void JSONMovieclipWriter::Init(Context&, SymbolContext& symbol, uint32_t frameCount) {
			m_frames.clear();

			for (uint32_t i = 0; frameCount > i; i++) {
				m_frames.push_back(
					{
						{"elements", json::array()}
					}
				);
			}

			if (symbol.hasSlice9)
			{
				DOM::Utils::RECT& rect = symbol.slice9;
				m_9scale = json::array({ rect.topLeft.y,  rect.bottomRight.y, rect.topLeft.x + rect.bottomRight.y, rect.bottomRight.x + rect.topLeft.y });
			}
		}

		void JSONMovieclipWriter::SetLabel(std::u16string& label) {
			m_frames.at(m_position)["label"] = label;
		}

		void JSONMovieclipWriter::AddFrameElement(
			uint16_t id,
			FCM::BlendMode blending,
			std::u16string name,
			DOM::Utils::MATRIX2D* matrix,
			DOM::Utils::COLOR_MATRIX* color
		) {
			ordered_json frameElement = {};
			frameElement["id"] = id;

			switch (blending)
			{
			case FCM::BlendMode::ADD_BLEND_MODE:
				frameElement["blend"] = "Add";
				break;
			case FCM::BlendMode::ALPHA_BLEND_MODE:
				frameElement["blend"] = "Alpha";
				break;
			case FCM::BlendMode::DARKEN_BLEND_MODE:
				frameElement["blend"] = "Darken";
				break;
			case FCM::BlendMode::DIFFERENCE_BLEND_MODE:
				frameElement["blend"] = "Difference";
				break;
			case FCM::BlendMode::ERASE_BLEND_MODE:
				frameElement["blend"] = "Erase";
				break;
			case FCM::BlendMode::HARDLIGHT_BLEND_MODE:
				frameElement["blend"] = "Hardlight";
				break;
			case FCM::BlendMode::INVERT_BLEND_MODE:
				frameElement["blend"] = "Invert";
				break;
			case FCM::BlendMode::LAYER_BLEND_MODE:
				frameElement["blend"] = "Layer";
				break;
			case FCM::BlendMode::LIGHTEN_BLEND_MODE:
				frameElement["blend"] = "Lighten";
				break;
			case FCM::BlendMode::MULTIPLY_BLEND_MODE:
				frameElement["blend"] = "Mulitply";
				break;
			case FCM::BlendMode::NORMAL_BLEND_MODE:
				frameElement["blend"] = "Normal";
				break;
			case FCM::BlendMode::OVERLAY_BLEND_MODE:
				frameElement["blend"] = "Overlay";
				break;
			case FCM::BlendMode::SCREEN_BLEND_MODE:
				frameElement["blend"] = "Screen";
				break;
			case FCM::BlendMode::SUBTRACT_BLEND_MODE:
				frameElement["blend"] = "Subtract";
				break;
			default:
				break;
			}

			if (!name.empty()) {
				frameElement["name"] = Utils::ToUtf8(name);
			}

			if (matrix != nullptr) {
				frameElement["matrix"] = {
					matrix->a,
					matrix->b,
					matrix->c,
					matrix->d,
					matrix->tx,
					matrix->ty
				};
			}

			if (color != nullptr) {
				frameElement["color"] = {
					{"rMul", color->matrix[0][0]},
					{"gMul", color->matrix[1][1]},
					{"bMul", color->matrix[2][2]},
					{"aMul", color->matrix[3][3]},

					{"rAdd", color->matrix[0][4]},
					{"gAdd", color->matrix[1][4]},
					{"bAdd", color->matrix[2][4]},
					{"aAdd", color->matrix[3][4]},
				};
			}

			// Pushing to "elements"
			m_frames.at(m_position)["elements"].push_back(frameElement);
		}

		void JSONMovieclipWriter::Finalize(uint16_t id, uint8_t fps) {
			ordered_json root = ordered_json::object(
				{
					{"id", id},
					{"exports",  json::array()},
					{"fps", fps},
					{"nineScale", m_9scale},
					{"frames", m_frames}
				});

			m_writer->AddMovieclip(root);
		}
	}
}