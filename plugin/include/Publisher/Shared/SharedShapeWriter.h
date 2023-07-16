#pragma once

#include <memory>

#include "FCMTypes.h"
#include "DOM/FrameElement/IFrameDisplayElement.h"

#include "DOM/LibraryItem/IMediaItem.h"

#include "Publisher/TimelineBuilder/FrameElements/FilledShape.h"

#include <opencv2/opencv.hpp>

namespace sc {
	namespace Adobe {
		class SharedShapeWriter {
		public:
			virtual void AddGraphic(const cv::Mat& image, const DOM::Utils::MATRIX2D& matrix) = 0;

			virtual void AddFilledShape(const FilledShape& shape) = 0; // TODO: nine slice

			virtual void Finalize(uint16_t id) = 0;
		};

		typedef std::shared_ptr<SharedShapeWriter> pSharedShapeWriter;
	}
}