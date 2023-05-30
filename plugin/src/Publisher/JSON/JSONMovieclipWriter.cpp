#include "Publisher/JSON/JSONMovieclipWriter.h"

#include "Publisher/JSON/JSONWriter.h"
#include "Macros.h"

using namespace FCM;

namespace sc {
	namespace Adobe {
		Result JSONMovieclipWriter::Init(JSONWriter* writer, PIFCMCallback callback) {
			Result res = FCM_SUCCESS;

			m_callback = callback;

			if (writer) {
				m_writer = writer;
			}
			else {
				return FCM_EXPORT_FAILED;
			}

			console.Init("MovieClip Writer", m_callback);

			return FCM_SUCCESS;
		}

		Result JSONMovieclipWriter::InitTimeline(U_Int32 frameCount) {
			m_frames->clear();

			for (U_Int32 i = 0; frameCount > i; i++) {
				JSONNode frame;

				JSONNode frameName("label", "");

				JSONNode frameElements(JSON_ARRAY);
				frameElements.set_name("elements");

				frame.push_back(frameName);
				frame.push_back(frameElements);

				m_frames->push_back(frame);
			}

			return FCM_SUCCESS;
		}

		Result JSONMovieclipWriter::SetLabel(U_Int32 frameIndex, std::string label) {
			m_frames->at(frameIndex)[0] = JSONNode("label", label);

			return FCM_SUCCESS;

		}

		Result JSONMovieclipWriter::AddFrameElement(
			U_Int32 frameIndex,
			U_Int16 id,
			U_Int8 blending,
			std::string name,
			DOM::Utils::MATRIX2D& matrix,
			DOM::Utils::COLOR_MATRIX& color
		) {
			JSONNode frameElement;

			frameElement.push_back(
				JSONNode("id", id)
			);

			frameElement.push_back(JSONNode("matrix", Utils::ToString(matrix)));

			JSONNode colorNode;
			colorNode.set_name("color");

			colorNode.push_back(JSONNode("rMul", color.matrix[0][0]));
			colorNode.push_back(JSONNode("gMul", color.matrix[1][1]));
			colorNode.push_back(JSONNode("bMul", color.matrix[2][2]));
			colorNode.push_back(JSONNode("aMul", color.matrix[3][3]));

			colorNode.push_back(JSONNode("rAdd", color.matrix[0][4]));
			colorNode.push_back(JSONNode("gAdd", color.matrix[1][4]));
			colorNode.push_back(JSONNode("bAdd", color.matrix[2][4]));
			colorNode.push_back(JSONNode("aAdd", color.matrix[3][4]));

			frameElement.push_back(colorNode);

			if (m_frames->size() < (frameIndex + 1)) {
				console.log("Failed to get frame %d", frameIndex);
				return FCM_EXPORT_FAILED;
			}

			// Pushing to "elements"
			m_frames->at(frameIndex)[1].push_back(frameElement);

			return FCM_SUCCESS;
		}

		void JSONMovieclipWriter::Finalize(U_Int16 id, U_Int8 fps, std::string name) {
			JSONNode root;

			root.push_back(
				JSONNode("id", id)
			);

			root.push_back(
				JSONNode("name", name)
			);

			root.push_back(
				JSONNode("fps", fps)
			);

			root.push_back(
				*m_frames
			);

			m_writer->AddMovieclip(root);

		}
	}
}