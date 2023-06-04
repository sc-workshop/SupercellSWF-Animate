#include "Publisher/JSON/JSONShapeWriter.h"

#include "Publisher/JSON/JSONWriter.h"
#include "Utils.h"

namespace sc {
	namespace Adobe {
		void JSONShapeWriter::Init(JSONWriter* writer, PIFCMCallback callback) {
			m_writer = writer;
			m_callback = callback;
		}

		void JSONShapeWriter::AddGraphic(cv::Mat& image, DOM::Utils::MATRIX2D matrix) {
			U_Int32 imageIndex = m_writer->imageCount;
			m_writer->imageCount++;
			
			std::string bitmapBasename = std::to_string(imageIndex) + ".png";
			fs::path bitmapOutputPath = m_writer->imageFolder / bitmapBasename;

			cv::imwrite(bitmapOutputPath.string(), image);

			JSONNode bitmap;

			bitmap.push_back(
				JSONNode("path", bitmapBasename)
			);

			bitmap.push_back(
				JSONNode("matrix", Utils::ToString(matrix))
			);

			m_bitmaps.push_back(bitmap);
		}

		void JSONShapeWriter::Finalize(U_Int16 id) {
			JSONNode root;

			root.push_back(
				JSONNode("id", id)
			);

			m_bitmaps.set_name("bitmaps");
			root.push_back(
				m_bitmaps
			);

			m_writer->AddShape(root);

		}
	}
}