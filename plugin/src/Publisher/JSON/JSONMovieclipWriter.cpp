#include "Publisher/JSON/JSONMovieclipWriter.h"

#include "Publisher/JSON/JSONWriter.h"

namespace sc {
	namespace Adobe {
		void JSONMovieclipWriter::Init(JSONWriter* writer) {
			if (writer) {
				m_writer = writer;
			}
			else {
				throw exception("Failed to get writer");
			}
		}

		void JSONMovieclipWriter::InitTimeline(uint32_t frameCount) {
			m_frames.clear();

			for (uint32_t i = 0; frameCount > i; i++) {
				m_frames.push_back({
					{"elements", json::array()}
				});
			}
		}

		void JSONMovieclipWriter::SetLabel(std::u16string label) {
			m_frames.at(m_position)["label"] = label;
		}

		void JSONMovieclipWriter::AddFrameElement(
			uint16_t id,
			uint8_t blending,
			std::u16string name,
			DOM::Utils::MATRIX2D* matrix,
			DOM::Utils::COLOR_MATRIX* color
		) {
			ordered_json frameElement = {};
			frameElement["id"] = id;

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
			ordered_json root = 
			{
				{"id", id},
				{"exports",  json::array()},
				{"fps", fps},
				{"frames", m_frames}
			};

			m_writer->AddMovieclip(root);

		}
	}
}