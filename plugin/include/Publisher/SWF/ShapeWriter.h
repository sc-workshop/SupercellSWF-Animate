#pragma once

#include "Publisher/Shared/SharedShapeWriter.h"
#include "Macros.h"
#include "ApplicationFCMPublicIDs.h"
#include "DOM/Service/Image/IBitmapExportService.h"
#include "string"

namespace sc {
	namespace Adobe {
		class Writer;

		class ShapeWriter : public SharedShapeWriter {
			PIFCMCallback m_callback = nullptr;
			Writer* m_writer = nullptr;
			uint16_t m_bitmapCount = 0;

			AutoPtr<DOM::Service::Image::IBitmapExportService> m_bitmapExportService = nullptr;

			const std::string tempFile = std::string(tmpnam(nullptr)) + ".png";

		public:
			~ShapeWriter() {
				remove(tempFile.c_str());
			};

			Result Init(Writer* writer, PIFCMCallback callback);
			Result AddGraphic(DOM::LibraryItem::IMediaItem* image, DOM::Utils::MATRIX2D matrix);

			void Finalize(U_Int16 id);
		};
	}
}