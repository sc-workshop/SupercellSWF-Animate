#include "Publisher/JSON/JSONShapeWriter.h"

#include "Publisher/JSON/JSONWriter.h"

namespace sc {
	namespace Adobe {
		Result JSONShapeWriter::Init(JSONWriter* writer, PIFCMCallback callback) {
			m_writer = writer;
			m_callback = callback;

			AutoPtr<IFCMUnknown> unknownService = nullptr;
			m_callback->GetService(DOM::FLA_BITMAP_SERVICE, unknownService.m_Ptr);
			m_bitmapExportService = unknownService;

			return FCM_SUCCESS;
		}

		Result JSONShapeWriter::AddGraphic(DOM::LibraryItem::IMediaItem* image) {
			U_Int32 imageIndex = m_writer->imageCount;
			m_writer->imageCount++;
			
			std::string bitmapBasename = std::to_string(imageIndex) + ".PNG";
			fs::path bitmapOutputPath = m_writer->imageFolder / bitmapBasename;

			m_bitmapExportService->ExportToFile(
				image,
				Utils::ToString16(bitmapOutputPath.string(), m_callback),
				100
			);

			JSONNode bitmap;

			bitmap.push_back(
				JSONNode("path", bitmapBasename)
			);

			m_bitmaps.push_back(bitmap);

			return FCM_SUCCESS;
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

			delete this;
		}
	}
}