#pragma once

// FCM stuff
#include "FCMTypes.h"
#include "DOM/Service/Tween/IPropertyIDs.h"
#include "Macros.h"

// Logger
#include "io/Console.h"

// Timeline stuff
#include <ITimeline.h>
#include <DOM/ILayer2.h>
#include <DOM/IFrame.h>
#include "DOM/ITween.h"

// Symbols on timeline
#include "DOM/FrameElement/IFrameDisplayElement.h"
#include "DOM/FrameElement/IInstance.h"

// Tween generator stuff
#include "DOM/Service/Tween/ITweenerService.h"
#include "DOM/Service/Tween/IGeometricTweener.h"
#include "DOM/Service/Tween/IColorTweener.h"

#include "Publisher/Shared/SharedMovieclipWriter.h"

using namespace FCM;
using namespace DOM::Service::Tween;

namespace sc {
	namespace Adobe {
		class ResourcePublisher;

		class FrameGenerator {
			PIFCMCallback m_callback = nullptr;
			ResourcePublisher* m_resources = nullptr;

			AutoPtr<ITweenerService> TweenerService = nullptr;

			Console console;

			// Helper functions

			Result InitializeService();

			Result GetDisplayInstanceData(
				DOM::FrameElement::IFrameDisplayElement* element,
				uint16_t& identifer,
				uint8_t& blending,
				std::string& name
			);

			Result GenerateFrame(
				pSharedMovieclipWriter writer,
				FCM::FCMListPtr frameElements,
				uint32_t frameElementsCount,
				uint32_t duration,
				uint32_t& frameOffset
			);

			Result GenerateTweenFrame(
				pSharedMovieclipWriter writer,
				DOM::ITween* tween,
				DOM::FrameElement::IFrameDisplayElement* element,
				uint32_t duration,
				uint32_t& frameOffset
			);

			Result GenerateLayerFrames(
				pSharedMovieclipWriter writer,
				AutoPtr<DOM::Layer::ILayerNormal> layer
			);

			Result GenerateLayer(
				pSharedMovieclipWriter writer,
				AutoPtr<DOM::ILayer2> layer
			);

			Result GenerateLayerList(
				pSharedMovieclipWriter writer,
				FCMListPtr layers
			);

		public:
			FrameGenerator() { };
			~FrameGenerator() { };

			Result Init(PIFCMCallback callback, ResourcePublisher* resources) {
				m_callback = callback;
				m_resources = resources;

				console.Init("FrameGenerator", m_callback);

				return FCM_SUCCESS;
			}

			Result Generate(pSharedMovieclipWriter writer, DOM::ITimeline* timeline);
		};
	};
}