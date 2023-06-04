#pragma once

#include "Publisher/Shared/SharedShapeWriter.h"
#include "Macros.h"
#include "ApplicationFCMPublicIDs.h"

#include "string"

namespace sc {
	namespace Adobe {
		class Writer;

		class ShapeWriter : public SharedShapeWriter {
			PIFCMCallback m_callback = nullptr;
			Writer* m_writer = nullptr;
			uint16_t m_bitmapCount = 0;

		public:
			void Init(Writer* writer, PIFCMCallback callback);
			void AddGraphic(cv::Mat& image, DOM::Utils::MATRIX2D matrix);

			void Finalize(U_Int16 id);
		};
	}
}