#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <tuple>

// FCM stuff
#include "FCMTypes.h"
#include "DOM/Service/Tween/IPropertyIDs.h"

// MATRIX2D, COLOR_MATRIX
#include "DOM/Utils/DOMTypes.h"

// Tweeners stuff
#include <DOM/ITween.h>
#include <DOM/Service/Tween/ITweenerService.h>
#include <DOM/Service/Tween/IGeometricTweener.h>
#include <DOM/Service/Tween/IColorTweener.h>

// Symbols
#include <DOM/ILibraryItem.h>
#include <DOM/FrameElement/IFrameDisplayElement.h>
#include <DOM/FrameElement/IInstance.h>
#include <DOM/FrameElement/ISymbolInstance.h>

// Timeline
#include <DOM/IFrame.h>

// Writer
#include "Publisher/Shared/SharedWriter.h"

// Fills
#include "FrameElements/FilledShape.h"

using namespace DOM::Service::Tween;

using DOM::Utils::MATRIX2D;
using DOM::Utils::COLOR_MATRIX;

namespace sc {
	namespace Adobe {
		class ResourcePublisher;

		class FrameBuilder {
			ResourcePublisher& m_resources;
			// Frame duration
			uint32_t m_duration = 0;

			// Local frame position
			uint32_t m_position = 0;
			// Frame name
			u16string m_label = u"";

			// Frane symbols data in resource palette
			vector<tuple<uint16_t, FCM::BlendMode, u16string>> m_elementsData;

			// Transforms for each frame element
			vector<shared_ptr<MATRIX2D>> m_matrices;
			vector<shared_ptr<COLOR_MATRIX>> m_colors;

			// Tweeners
			FCM::AutoPtr<DOM::ITween> m_tween = nullptr;

			FCM::AutoPtr<IGeometricTweener> m_matrixTweener = nullptr;
			FCM::AutoPtr<IColorTweener> m_colorTweener = nullptr;

		public:
			FrameBuilder(ResourcePublisher& resources) : m_resources(resources) { };

			void Update(FCM::AutoPtr<DOM::IFrame>& frame);

			void operator()(pSharedMovieclipWriter writer);

			bool empty() {
				return m_elementsData.size() == 0;
			}

			void next() {
				m_position++;
			}

			operator bool() const
			{
				return m_duration > m_position;
			}
		};
	}
}