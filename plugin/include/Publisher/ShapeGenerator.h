#pragma once

#include "io/Console.h"
#include "Macros.h"

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

using namespace FCM;
using namespace std;

namespace sc {
	namespace Adobe {
		class ResourcePublisher;

		class ShapeGenerator {
			PIFCMCallback m_callback;
			ResourcePublisher& m_resources;

			Console console;

			AutoPtr<DOM::Service::Image::IBitmapExportService> BitmapExportService = nullptr;

			void InitializeService();

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
			ShapeGenerator(PIFCMCallback callback, ResourcePublisher& resources):
			m_callback(callback),
			m_resources(resources)
			{
				console.Init("ShapeGenerator", m_callback);
				InitializeService();
			};
			~ShapeGenerator() {
				if (fs::exists(tempFile)) {
					remove(tempFile);
				}
			};

			const fs::path tempFile = fs::path(tmpnam(nullptr)).concat(".png");

			void GetImage(AutoPtr<DOM::LibraryItem::IMediaItem>& media, cv::Mat& image);

			void Generate(pSharedShapeWriter writer, DOM::ITimeline* timeline);

			static bool Validate(DOM::ITimeline* timeline);
		};
	}
}