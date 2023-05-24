#include "Publisher/SWF/ShapeWriter.h"
#include "Publisher/SWF/Writer.h"
#include "Utils.h"

#include "opencv2/opencv.hpp"
using namespace sc;

namespace sc {
	namespace Adobe {
		Result ShapeWriter::Init(Writer* writer, PIFCMCallback callback) {
			m_writer = writer;
			m_callback = callback;

			AutoPtr<IFCMUnknown> unknownService = nullptr;
			m_callback->GetService(DOM::FLA_BITMAP_SERVICE, unknownService.m_Ptr);
			m_bitmapExportService = unknownService;

			return FCM_SUCCESS;
		}

		Result ShapeWriter::AddGraphic(DOM::LibraryItem::IMediaItem* adobeImage, DOM::Utils::MATRIX2D matrix) {
			Result res;

			res = m_bitmapExportService->ExportToFile(
				adobeImage,
				Utils::ToString16(tempFile, m_callback),
				100
			);
			FCM_CHECK;

			Sprite sprite;
			sprite.image = cv::imread(tempFile, cv::IMREAD_UNCHANGED);
			sprite.matrix = matrix;

			m_writer->sprites.push_back(sprite);
			m_bitmapCount++;

			return FCM_SUCCESS;
		}

		void ShapeWriter::Finalize(U_Int16 id) {
			pShape shape = pShape(new Shape());
			shape->id(id);
			shape->commands.resize(m_bitmapCount);

			m_writer->swf.shapes.push_back(shape);
			delete this;
		}
	}
}