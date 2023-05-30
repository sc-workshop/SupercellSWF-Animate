#pragma once

#include "Publisher/Shared/SharedShapeWriter.h"

// FCM stuff
#include "FCMTypes.h"
#include "Macros.h"

// Logger
#include "io/Console.h"

// Timeline stuff
#include <ITimeline.h>
#include <DOM/ILayer2.h>
#include <DOM/IFrame.h>
#include "DOM/ITween.h"

// Image export service
#include "DOM/Service/Image/IBitmapExportService.h"

using namespace FCM;

namespace sc {
	namespace Adobe {
		class ResourcePublisher;

		class ShapeGenerator {
			PIFCMCallback m_callback = nullptr;
			ResourcePublisher* m_resources = nullptr;

			Console console;

			AutoPtr<DOM::Service::Image::IBitmapExportService> BitmapExportService = nullptr;

			Result InitializeService();

			Result GenerateLayerShapes(
				pSharedShapeWriter writer,
				AutoPtr<DOM::Layer::ILayerNormal> layer
			);

			Result GenerateLayer(
				pSharedShapeWriter writer,
				AutoPtr<DOM::ILayer2> layer
			);

			Result GenerateLayerList(
				pSharedShapeWriter writer,
				FCMListPtr layers
			);

			// Validate Stuff

			static Result ValidateLayerItems(
				AutoPtr<DOM::Layer::ILayerNormal> layer,
				bool& result
			);

			static Result ValidateLayer(
				AutoPtr<DOM::ILayer2> layer,
				bool& result
			);

			static Result ValidateLayerList(
				FCMListPtr layers,
				bool& result
			);

		public:
			ShapeGenerator() { };
			~ShapeGenerator() {
				if (fs::exists(tempFile)) {
					remove(tempFile);
				}
			};

			const fs::path tempFile = fs::path(tmpnam(nullptr)).concat(".png");

			Result GetImage(AutoPtr<DOM::LibraryItem::IMediaItem>& media, cv::Mat& image);

			Result Init(PIFCMCallback callback, ResourcePublisher* resources) {
				m_callback = callback;
				m_resources = resources;

				console.Init("ShapeGenerator", m_callback);

				return FCM_SUCCESS;
			}

			Result Generate(pSharedShapeWriter writer, DOM::ITimeline* timeline);

			static Result Validate(DOM::ITimeline* timeline, bool& result);
		};
	}
}