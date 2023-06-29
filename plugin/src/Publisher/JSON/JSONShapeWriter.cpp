#include "Publisher/JSON/JSONShapeWriter.h"

#include "Publisher/JSON/JSONWriter.h"
#include "Utils.h"

namespace sc {
	namespace Adobe {
		void JSONShapeWriter::Init(JSONWriter* writer) {
			if (writer) {
				m_writer = writer;
			}
			else {
				throw exception("Failed to get writer");
			}
		}

		void JSONShapeWriter::AddGraphic(cv::Mat& image, DOM::Utils::MATRIX2D matrix) {
			uint32_t imageIndex = m_writer->imageCount;
			m_writer->imageCount++;
			
			std::string bitmapBasename = std::to_string(imageIndex) + ".png";
			fs::path bitmapOutputPath = m_writer->imageFolder / bitmapBasename;

			cv::imwrite(bitmapOutputPath.string(), image);

			m_bitmaps.push_back({
				{"path", bitmapBasename},
				{"matrix", Utils::ToString(matrix)}
			});
		}

		void JSONShapeWriter::Finalize(uint16_t id) {
			ordered_json root = {
				{"id", id},
				{"bitmaps", m_bitmaps}
			};

			m_writer->AddShape(root);

		}
	}
}