#include "Publisher/SWF/Writer.h"
#include "Publisher/SWF/MovieclipWriter.h"

#include "Macros.h"

using namespace FCM;

namespace sc {
	namespace Adobe {
		Result MovieclipWriter::Init(Writer* writer, PIFCMCallback callback) {
			Result res = FCM_SUCCESS;

			m_callback = callback;

			if (writer) {
				m_writer = writer;
			}
			else {
				return FCM_EXPORT_FAILED;
			}

			console.Init("Timeline", m_callback);

			return FCM_SUCCESS;
		}

		Result MovieclipWriter::InitTimeline(U_Int32 frameCount) {
			return FCM_SUCCESS;
		}

		Result MovieclipWriter::SetLabel(U_Int32 frameIndex, std::string label) {
			return FCM_SUCCESS;
		}

		Result MovieclipWriter::AddFrameElement(
			U_Int32 frameIndex,
			U_Int16 id,
			U_Int8 blending,
			std::string name,
			DOM::Utils::MATRIX2D* matrix,
			DOM::Utils::COLOR_MATRIX* color
		) {
			return FCM_SUCCESS;
		}

		void MovieclipWriter::Finalize(U_Int16 id, U_Int8 fps, std::string name) {

			delete this;
		}
	}
}