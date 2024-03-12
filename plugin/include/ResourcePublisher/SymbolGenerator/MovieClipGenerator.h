#pragma once

#include <vector>

// FCM stuff
#include <AnimateSDK/core/common/FCMTypes.h>
#include <AnimateSDK/app/DOM/Service/Tween/IPropertyIDs.h>

// Timeline
#include <AnimateSDK/app/DOM/ITimeline1.h>
#include <AnimateSDK/app/DOM/ILayer2.h>

// Writer
#include "ResourcePublisher/Writer/Shared/SharedMovieclipWriter.h"

#include "TimelineBuilder/LayerBuilder.h"

namespace sc {
	namespace Adobe {
		class ResourcePublisher;

		class MovieClipGeneator {
			ResourcePublisher& m_resources;

		public:
			static void GetLayerBuilder(FCM::FCMListPtr& layers, ResourcePublisher& resources, SymbolContext& symbol, std::vector<LayerBuilder>& result);

			MovieClipGeneator(ResourcePublisher& resources) : m_resources(resources) {}

			void Generate(SharedMovieclipWriter& writer, SymbolContext& symbol, FCM::AutoPtr<DOM::ITimeline1> timeline);
		};
	}
}