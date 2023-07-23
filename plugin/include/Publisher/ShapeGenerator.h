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

using namespace DOM::Service::Image;

namespace sc {
	namespace Adobe {
		class ResourcePublisher;

		class ShapeGenerator {
			ResourcePublisher& m_resources;
			FCM::AutoPtr<IBitmapExportService> BitmapExportService;

			void GenerateLayerElements(
				pSharedShapeWriter writer,
				FCM::FCMListPtr elements
			);

			void GenerateLayerShapes(
				pSharedShapeWriter writer,
				FCM::AutoPtr<DOM::Layer::ILayerNormal> layer
			);

			void GenerateLayer(
				pSharedShapeWriter writer,
				FCM::AutoPtr<DOM::ILayer2> layer
			);

			void GenerateLayerList(
				pSharedShapeWriter writer,
				FCM::FCMListPtr layers
			);

			// Validate Stuff

			static bool ValidateLayerFrameElements(FCM::FCMListPtr frameElements);

			static bool ValidateLayerItems(
				FCM::AutoPtr<DOM::Layer::ILayerNormal> layer
			);

			static bool ValidateLayer(
				FCM::AutoPtr<DOM::ILayer2> layer
			);

			static bool ValidateLayerList(
				FCM::FCMListPtr layers
			);

		public:
			ShapeGenerator(ResourcePublisher& resources);;

			~ShapeGenerator();

			const fs::path tempFile = fs::path(tmpnam(nullptr)).concat(".png");

			void GetImage(FCM::AutoPtr<DOM::LibraryItem::IMediaItem>& media, cv::Mat& image);

			void Generate(pSharedShapeWriter writer, DOM::ITimeline* timeline);

			static bool Validate(DOM::ITimeline* timeline);
		};
	}
}