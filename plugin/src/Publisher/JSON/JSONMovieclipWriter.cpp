#include "Publisher/JSON/JSONMovieclipWriter.h"

#include "Publisher/JSON/JSONWriter.h"
#include "Macros.h"

using namespace FCM;

namespace sc {
	namespace Adobe {
		void JSONMovieclipWriter::Init(JSONWriter* writer, PIFCMCallback callback) {
			m_callback = callback;

			if (writer) {
				m_writer = writer;
			}
			else {
				return;
			}

			console.Init("[JSON] MovieClip", m_callback);
		}

		void JSONMovieclipWriter::InitTimeline(uint32_t frameCount) {
			m_frames.clear();

			for (uint32_t i = 0; frameCount > i; i++) {
				JSONNode frame;

				JSONNode frameName("label", "");

				JSONNode frameElements(JSON_ARRAY);
				frameElements.set_name("elements");

				frame.push_back(frameName);
				frame.push_back(frameElements);

				m_frames.push_back(frame);
			}
		}

		void JSONMovieclipWriter::SetLabel(std::string label) {
			m_frames.at(m_position)[0] = JSONNode("label", label);
		}

		void JSONMovieclipWriter::AddFrameElement(
			uint16_t id,
			uint8_t blending,
			std::string name,
			DOM::Utils::MATRIX2D* matrix,
			DOM::Utils::COLOR_MATRIX* color
		) {
			JSONNode frameElement;

			frameElement.push_back(
				JSONNode("id", id)
			);

			if (matrix != nullptr) {
				JSONNode matrixNode;
				matrixNode.set_name("matrix");
				matrixNode.push_back(JSONNode("a", matrix->a));
				matrixNode.push_back(JSONNode("b", matrix->b));
				matrixNode.push_back(JSONNode("c", matrix->c));
				matrixNode.push_back(JSONNode("d", matrix->d));
				matrixNode.push_back(JSONNode("tx", matrix->tx));
				matrixNode.push_back(JSONNode("ty", matrix->ty));
				frameElement.push_back(matrixNode);
			}
			
			if (color != nullptr) {
				JSONNode colorNode;
				colorNode.set_name("color");

				colorNode.push_back(JSONNode("rMul", color->matrix[0][0]));
				colorNode.push_back(JSONNode("gMul", color->matrix[1][1]));
				colorNode.push_back(JSONNode("bMul", color->matrix[2][2]));
				colorNode.push_back(JSONNode("aMul", color->matrix[3][3]));

				colorNode.push_back(JSONNode("rAdd", color->matrix[0][4]));
				colorNode.push_back(JSONNode("gAdd", color->matrix[1][4]));
				colorNode.push_back(JSONNode("bAdd", color->matrix[2][4]));
				colorNode.push_back(JSONNode("aAdd", color->matrix[3][4]));

				frameElement.push_back(colorNode);
			}

			// Pushing to "elements"
			m_frames.at(m_position)[1].push_back(frameElement);
		}

		void JSONMovieclipWriter::Finalize(uint16_t id, uint8_t fps, u16string name) {
			JSONNode root;

			root.push_back(
				JSONNode("id", id)
			);

			fs::path symbolPath(name);
			root.push_back(
				JSONNode("name", symbolPath.filename().string())
			);

			root.push_back(
				JSONNode("fps", fps)
			);

			m_frames.set_name("frames");
			root.push_back(
				m_frames
			);

			m_writer->AddMovieclip(root);

		}
	}
}