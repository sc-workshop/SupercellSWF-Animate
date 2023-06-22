#pragma once

#include <memory>

#include "FCMTypes.h"
#include "DOM/FrameElement/IFrameDisplayElement.h"

#include "DOM/LibraryItem/IMediaItem.h"

#include <opencv2/opencv.hpp>

namespace sc {
	namespace Adobe {
		class SharedShapeWriter {
		public:
			virtual void AddGraphic(cv::Mat& image, DOM::Utils::MATRIX2D matrix) = 0;

			virtual void Finalize(uint16_t id) = 0;
		};

		typedef std::shared_ptr<SharedShapeWriter> pSharedShapeWriter;
	}
}