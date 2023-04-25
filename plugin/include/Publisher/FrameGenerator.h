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

using namespace FCM;
using namespace DOM::Service::Tween;

namespace sc {
	namespace Adobe {
		class SharedMovieclipWriter;
		class ResourcePublisher;

		class FrameGenerator {
			PIFCMCallback m_callback = nullptr;
			ResourcePublisher* m_resources = nullptr;

			AutoPtr<ITweenerService> TweenerService = nullptr;

			Console console;

			// Helper functions

			Result InitializeTweenerService();

			Result GetDisplayInstanceData(
				DOM::FrameElement::IFrameDisplayElement* element,
				U_Int16& identifer,
				U_Int8& blending,
				std::string& name
			);

			Result GenerateTweenFrames(
				SharedMovieclipWriter* writer,
				DOM::ITween* tween,
				DOM::FrameElement::IFrameDisplayElement* element,
				U_Int32 duration,
				U_Int32& frameOffset
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

			Result Generate(SharedMovieclipWriter* writer, DOM::ITimeline* timeline);
		};
	};
}