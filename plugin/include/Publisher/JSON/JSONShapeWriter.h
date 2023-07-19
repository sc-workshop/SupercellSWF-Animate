#pragma once

#include "Publisher/Shared/SharedShapeWriter.h"

#include "ApplicationFCMPublicIDs.h"
#include "DOM/Service/Image/IBitmapExportService.h"

#include <filesystem>
namespace fs = std::filesystem;

#include "JSON.hpp"
using namespace nlohmann;

namespace sc {
	namespace Adobe {
		class JSONWriter;

		class JSONShapeWriter : public SharedShapeWriter {
			JSONWriter* m_writer = nullptr;

			json m_bitmaps = json::array();

		public:
			void Init(JSONWriter* writer);
			void AddGraphic(const cv::Mat& image, const DOM::Utils::MATRIX2D& matrix);
			void AddFilledShape(const FilledShape& shape, bool isNineSlice);

			void Finalize(uint16_t id);
		};
	}
}