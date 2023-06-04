#pragma once

#include "Publisher/Shared/SharedShapeWriter.h"

#include "ApplicationFCMPublicIDs.h"
#include "DOM/Service/Image/IBitmapExportService.h"

#include <filesystem>
namespace fs = std::filesystem;

#include "libjson.h"

namespace sc {
	namespace Adobe {
		class JSONWriter;

		class JSONShapeWriter : public SharedShapeWriter {
			PIFCMCallback m_callback = nullptr;
			JSONWriter* m_writer = nullptr;

			JSONNode m_bitmaps = JSONNode(JSON_ARRAY);

		public:
			void Init(JSONWriter* writer, PIFCMCallback callback);
			void AddGraphic(cv::Mat& image, DOM::Utils::MATRIX2D matrix);

			void Finalize(U_Int16 id);
		};
	}
}