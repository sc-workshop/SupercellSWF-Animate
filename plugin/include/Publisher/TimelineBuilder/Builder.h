#pragma once

#include <vector>

// FCM stuff
#include <FCMTypes.h>
#include <DOM/Service/Tween/IPropertyIDs.h>

// Timeline
#include <DOM/ITimeline.h>
#include <DOM/ILayer2.h>

// Writer
#include "Publisher/Shared/SharedWriter.h"

#include "LayerBuilder.h"

using namespace FCM;

namespace sc {
	namespace Adobe {
		class ResourcePublisher;

		class TimelineBuilder {
			ResourcePublisher& m_resources;

		public:
			static void GetLayerBuilder(FCMListPtr& layers, ResourcePublisher& resources, vector<LayerBuilder>& result);

			TimelineBuilder(ResourcePublisher& resources): m_resources(resources) {}

			void Generate(pSharedMovieclipWriter writer, AutoPtr<DOM::ITimeline> timeline);
		};
	}
}