#pragma once

// FCM stuff
#include "AnimateSDK/core/common/FCMTypes.h"
#include "AnimateSDK/app/DOM/Service/Image/IBitmapExportService.h"

// Timeline stuff
#include <AnimateSDK/app/DOM/ITimeline.h>
#include <AnimateSDK/app/DOM/ILayer2.h>
#include <AnimateSDK/app/DOM/IFrame.h>
#include "AnimateSDK/app/DOM/ITween.h"

// Symbol
#include <AnimateSDK/app/DOM/ILibraryItem.h>
#include <AnimateSDK/app/DOM/MediaInfo/IBitmapInfo.h>
#include <AnimateSDK/app/DOM/FrameElement/IInstance.h>

// Writer
#include "ResourcePublisher/Writer/Shared/SharedGraphicWriter.h"

#include "Module/SymbolContext.h"

#include <filesystem>
namespace fs = std::filesystem;

using namespace DOM::Service::Image;

namespace sc {
	namespace Adobe {
		class ResourcePublisher;
		class PluginContext;

		class GraphicGenerator {
			ResourcePublisher& m_resources;

			void GenerateLayerElements(
				SymbolContext& symbol,
				SharedShapeWriter& writer,
				FCM::FCMListPtr elements
			);

			void GenerateLayerShapes(
				SymbolContext& symbol,
				SharedShapeWriter& writer,
				FCM::AutoPtr<DOM::Layer::ILayerNormal> layer
			);

			void GenerateLayer(
				SymbolContext& symbol,
				SharedShapeWriter& writer,
				FCM::AutoPtr<DOM::ILayer2> layer
			);

			void GenerateLayerList(
				SymbolContext& symbol,
				SharedShapeWriter& writer,
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
			GraphicGenerator(ResourcePublisher& resources);;

			~GraphicGenerator();

			void Generate(SymbolContext& symbol, SharedShapeWriter& writer, DOM::ITimeline* timeline);

			static bool Validate(DOM::ITimeline* timeline);
		};
	}
}