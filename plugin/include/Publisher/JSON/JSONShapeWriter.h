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

			AutoPtr<DOM::Service::Image::IBitmapExportService> m_bitmapExportService = nullptr;

			JSONNode m_bitmaps = JSONNode(JSON_ARRAY);

		public:
			Result Init(JSONWriter* writer, PIFCMCallback callback);
			Result AddGraphic(DOM::LibraryItem::IMediaItem* image);

			void Finalize(U_Int16 id);
		};
	}
}