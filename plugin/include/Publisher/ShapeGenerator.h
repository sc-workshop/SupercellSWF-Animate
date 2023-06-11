#pragma once

// FCM stuff
#include "FCMTypes.h"
#include "DOM/Service/Image/IBitmapExportService.h"

// Timeline stuff
#include <ITimeline.h>
#include <DOM/ILayer2.h>
#include <DOM/IFrame.h>
#include "DOM/ITween.h"

// Symbol
#include <DOM/ILibraryItem.h>
#include <DOM/MediaInfo/IBitmapInfo.h>
#include <DOM/FrameElement/IInstance.h>

// Writer
#include "Publisher/Shared/SharedShapeWriter.h"

#include <filesystem>
namespace fs = std::filesystem;

using namespace FCM;
using namespace std;
using namespace DOM::Service::Image;

namespace sc {
	namespace Adobe {
		class ResourcePublisher;

		class ShapeGenerator {
			ResourcePublisher& m_resources;
			AutoPtr<IBitmapExportService> BitmapExportService;

			void GenerateLayerShapes(
				pSharedShapeWriter writer,
				AutoPtr<DOM::Layer::ILayerNormal> layer
			);

			void GenerateLayer(
				pSharedShapeWriter writer,
				AutoPtr<DOM::ILayer2> layer
			);

			void GenerateLayerList(
				pSharedShapeWriter writer,
				FCMListPtr layers
			);

			// Validate Stuff

			static bool ValidateLayerItems(
				AutoPtr<DOM::Layer::ILayerNormal> layer
			);

			static bool ValidateLayer(
				AutoPtr<DOM::ILayer2> layer
			);

			static bool ValidateLayerList(
				FCMListPtr layers
			);

		public:
			ShapeGenerator(ResourcePublisher& resources);;

			~ShapeGenerator();

			const fs::path tempFile = fs::path(tmpnam(nullptr)).concat(".png");

			void GetImage(AutoPtr<DOM::LibraryItem::IMediaItem>& media, cv::Mat& image);

			void Generate(pSharedShapeWriter writer, DOM::ITimeline* timeline);

			static bool Validate(DOM::ITimeline* timeline);
		};
	}
}