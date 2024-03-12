#pragma once

#include <cstdint>
#include <vector>

// FCM stuff
#include <AnimateSDK/core/common/FCMTypes.h>
#include <AnimateSDK/app/DOM/Service/Tween/IPropertyIDs.h>

// Timeline
#include <AnimateSDK/app/DOM/ILayer2.h>
#include <AnimateSDK/app/DOM/FrameElement/IMovieClip.h>

// Modifier
#include <SupercellFlash/objects/MovieClipModifier.h>

// Writer
#include "ResourcePublisher/Writer/Shared/SharedWriter.h"

#include "FrameBuilder.h"
#include "Module/SymbolContext.h"

namespace sc {
	namespace Adobe {
		class ResourcePublisher;

		class LayerBuilder {
			SymbolContext& m_symbol;

			uint32_t m_duration = 0;
			uint32_t m_position = 0;

			FCM::FCMListPtr m_keyframes;
			uint32_t m_keyframeCount = 0;
			uint32_t m_keyframeIndex = 0;

			FCM::AutoPtr<DOM::Layer::ILayerNormal> m_layer;
			ResourcePublisher& m_resources;

			FrameBuilder m_frameBuilder;
			std::vector<LayerBuilder> m_maskedLayers;

			void UpdateFrame(SymbolContext& symbol);

			void AddModifier(
				SharedMovieclipWriter& writer,
				MaskedLayerState type
			);

		public:
			LayerBuilder(FCM::AutoPtr<DOM::Layer::ILayerNormal> layer, ResourcePublisher& resources, SymbolContext& info);

			void next();

			void operator()(SymbolContext& symbol, SharedMovieclipWriter& writer);

			operator bool() const
			{
				return m_duration > m_position;
			}
		};
	}
}