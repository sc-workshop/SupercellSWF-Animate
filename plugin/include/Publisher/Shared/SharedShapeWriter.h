#pragma once

#include <memory>

#include "FCMTypes.h"
#include "DOM/FrameElement/IFrameDisplayElement.h"

#include "DOM/LibraryItem/IMediaItem.h"

#include <opencv2/opencv.hpp>

using namespace FCM;

namespace sc {
	namespace Adobe {
		class SharedShapeWriter {
		public:
			virtual Result AddGraphic(cv::Mat& image, DOM::Utils::MATRIX2D matrix) = 0;

			virtual void Finalize(U_Int16 id) = 0;
		};

		typedef std::shared_ptr<SharedShapeWriter> pSharedShapeWriter;
	}
}