#pragma once

#include <vector>

#include "io/Console.h"
#include "Macros.h"

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
			PIFCMCallback m_callback;

			Console console;
		public:
			static void GetLayerBuilder(FCMListPtr& layers, ResourcePublisher& resources, vector<LayerBuilder>& result);

			TimelineBuilder(PIFCMCallback callback, ResourcePublisher& resources): m_callback(callback), m_resources(resources) {
				console.Init("TimelineBuilder", m_callback);
			}

			void Generate(pSharedMovieclipWriter writer, AutoPtr<DOM::ITimeline> timeline);
		};
	}
}