#pragma once

#include "FCMTypes.h"
#include "DOM/FrameElement/IFrameDisplayElement.h"

#include "DOM/LibraryItem/IMediaItem.h"

using namespace FCM;

namespace sc {
	namespace Adobe {
		class SharedShapeWriter {
		public:
			virtual Result AddGraphic(DOM::LibraryItem::IMediaItem* image) = 0;

			virtual void Finalize(U_Int16 id) = 0;
		};
	}
}