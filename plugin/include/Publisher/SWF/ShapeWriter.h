#pragma once

#include "Publisher/Shared/SharedShapeWriter.h"
#include "ApplicationFCMPublicIDs.h"

#include "string"

namespace sc {
	namespace Adobe {
		class Writer;

		class ShapeWriter : public SharedShapeWriter {
			Writer* m_writer = nullptr;
			uint16_t m_bitmapCount = 0;

		public:
			void Init(Writer* writer);
			void AddGraphic(cv::Mat& image, DOM::Utils::MATRIX2D matrix);

			void Finalize(uint16_t id);
		};
	}
}