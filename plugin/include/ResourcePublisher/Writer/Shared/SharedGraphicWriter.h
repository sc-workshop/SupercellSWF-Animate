#pragma once

#include <memory>
#include <opencv2/opencv.hpp>

#include "AnimateSDK/core/common/FCMTypes.h"
#include "AnimateSDK/app/DOM/FrameElement/IFrameDisplayElement.h"

#include "AnimateSDK/app/DOM/LibraryItem/IMediaItem.h"

#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledShape.h"
#include "Module/SymbolContext.h"

namespace sc {
	namespace Adobe {
		class SharedShapeWriter {
		public:
			virtual ~SharedShapeWriter() = default;

		public:
			virtual void AddGraphic(const cv::Mat& image, const DOM::Utils::MATRIX2D& matrix) = 0;

			virtual void AddFilledShape(const FilledShape& shape) = 0;

			virtual void Finalize(uint16_t id) = 0;
		};
	}
}