#pragma once

#include <memory>

#include "AnimateSDK/core/common/FCMTypes.h"
#include "AnimateSDK/app/DOM/FrameElement/IFrameDisplayElement.h"

#include "AnimateSDK/app/DOM/LibraryItem/IMediaItem.h"

#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledElement.h"
#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/SpriteElement.h"
#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/SliceElement.h"
#include "Module/Symbol/SymbolContext.h"

namespace sc {
	namespace Adobe {
		class SharedShapeWriter {
		public:
			virtual ~SharedShapeWriter() = default;

		public:
			virtual void AddGraphic(const SpriteElement& item, const DOM::Utils::MATRIX2D& matrix) = 0;

			virtual void AddFilledElement(const FilledElement& shape) = 0;

			virtual void AddSlicedElements(const std::vector<FilledElement>& elements, const DOM::Utils::RECT& guides) = 0;

			virtual bool Finalize(uint16_t id) = 0;
		};
	}
}