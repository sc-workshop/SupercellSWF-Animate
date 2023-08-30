#pragma once

#include <memory>
#include <opencv2/opencv.hpp>

#include "FCMTypes.h"
#include "DOM/FrameElement/IFrameDisplayElement.h"

#include "DOM/LibraryItem/IMediaItem.h"

#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledShape.h"
#include "ResourcePublisher/SymbolGenerator/SymbolBehavior.h"

namespace sc {
	namespace Adobe {
		class Context;

		class SharedShapeWriter {
		public:
			virtual void Init(Context& context, SymbolBehaviorInfo& info) = 0;

			virtual void AddGraphic(const cv::Mat& image, const DOM::Utils::MATRIX2D& matrix) = 0;

			virtual void AddFilledShape(const FilledShape& shape) = 0;

			virtual void Finalize(uint16_t id) = 0;
		};

		typedef std::shared_ptr<SharedShapeWriter> pSharedShapeWriter;
	}
}