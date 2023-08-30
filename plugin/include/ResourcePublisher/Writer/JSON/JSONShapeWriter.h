#pragma once

#include <filesystem>
namespace fs = std::filesystem;

#include "ResourcePublisher/Writer/Shared/SharedGraphicWriter.h"

#include "ApplicationFCMPublicIDs.h"
#include "DOM/Service/Image/IBitmapExportService.h"

#include "json.hpp"
using namespace nlohmann;

namespace sc {
	namespace Adobe {
		class JSONWriter;

		class JSONShapeWriter : public SharedShapeWriter {
			JSONWriter* m_writer = nullptr;

			ordered_json m_bitmaps = json::array();

		public:
			JSONShapeWriter(JSONWriter* writer) : m_writer(writer) {};

			void Init(Context& context, SymbolBehaviorInfo& info);
			void AddGraphic(const cv::Mat& image, const DOM::Utils::MATRIX2D& matrix);
			void AddFilledShape(const FilledShape& shape);

			void Finalize(uint16_t id);
		};
	}
}