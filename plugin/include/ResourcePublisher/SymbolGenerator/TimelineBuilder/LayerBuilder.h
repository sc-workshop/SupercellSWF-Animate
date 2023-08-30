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
#include "ResourcePublisher/Writer/Shared/SharedWriter.h"

#include "FrameBuilder.h"
#include "ResourcePublisher/SymbolGenerator/SymbolBehavior.h"

namespace sc {
	namespace Adobe {
		class ResourcePublisher;

		class LayerBuilder {
			SymbolBehaviorInfo& m_behavior;

			uint32_t m_duration = 0;
			uint32_t m_position = 0;

			FCM::FCMListPtr m_keyframes;
			uint32_t m_keyframeCount = 0;
			uint32_t m_keyframeIndex = 0;

			FCM::AutoPtr<DOM::Layer::ILayerNormal> m_layer;
			ResourcePublisher& m_resources;

			FrameBuilder m_frameBuilder;
			vector<LayerBuilder> m_maskedLayers;

			void UpdateFrame();

			void AddModifier(
				pSharedMovieclipWriter writer,
				sc::MovieClipModifier::Type type
			);

		public:
			LayerBuilder(FCM::AutoPtr<DOM::Layer::ILayerNormal> layer, ResourcePublisher& resources, SymbolBehaviorInfo& info);

			void next();

			void operator()(pSharedMovieclipWriter writer);

			operator bool() const
			{
				return m_duration > m_position;
			}
		};
	}
}