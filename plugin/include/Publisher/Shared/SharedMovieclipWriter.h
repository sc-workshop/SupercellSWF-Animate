#pragma once

#include <vector>
#include <string>
#include <memory>

#include "FCMTypes.h"
#include "DOM/FrameElement/IFrameDisplayElement.h"

using namespace FCM;

namespace sc {
	namespace Adobe {
		class SharedMovieclipWriter {
		public:
			virtual Result InitTimeline(U_Int32 frameCount) = 0;

			virtual Result SetLabel(U_Int32 frameIndex, std::string label) = 0;

			virtual Result AddFrameElement(U_Int32 frameIndex, U_Int16 id, U_Int8 blending, std::string name, DOM::Utils::MATRIX2D& matrix, DOM::Utils::COLOR_MATRIX& color) = 0;

			virtual void Finalize(U_Int16 id, U_Int8 fps, std::string name) = 0;
		};

		typedef std::shared_ptr<SharedMovieclipWriter> pSharedMovieclipWriter;
	}
}