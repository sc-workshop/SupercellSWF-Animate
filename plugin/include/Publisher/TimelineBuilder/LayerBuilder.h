#pragma once

#include <cstdint>
#include <vector>

// FCM stuff
#include <FCMTypes.h>
#include <DOM/Service/Tween/IPropertyIDs.h>

// Timeline
#include <DOM/ILayer2.h>
#include <DOM/FrameElement/IMovieClip.h>

// Modifier
#include <SupercellFlash/objects/MovieClipModifier.h>

// Writer
#include "Publisher/Shared/SharedWriter.h"

#include "FrameBuilder.h"

using namespace FCM;
using namespace std;

namespace sc {
	namespace Adobe {
		class ResourcePublisher;

		class LayerBuilder {
			uint32_t m_duration = 0;
			uint32_t m_position = 0;
			
			AutoPtr<DOM::Layer::ILayerNormal> m_layer;
			ResourcePublisher& m_resources;

			FrameBuilder m_frameBuilder;
			vector<LayerBuilder> m_maskedLayers;

			void UpdateFrame();

			void AddModifier(
				pSharedMovieclipWriter writer,
				sc::MovieClipModifier::Type type
			);

		public:
			LayerBuilder(AutoPtr<DOM::Layer::ILayerNormal> layer, ResourcePublisher& resources);

			void next();

			void operator()(pSharedMovieclipWriter writer);

			operator bool() const
			{
				return m_duration > m_position;
			}
		};
	}
}