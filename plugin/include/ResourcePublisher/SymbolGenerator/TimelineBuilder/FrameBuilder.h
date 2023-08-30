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
#include "DOM/ITween.h"
#include "DOM/Service/Tween/ITweenerService.h"
#include "DOM/Service/Tween/IGeometricTweener.h"
#include "DOM/Service/Tween/IColorTweener.h"
#include "DOM/Service/Tween/IShapeTweener.h"

// Symbols
#include "DOM/ILibraryItem.h"
#include "DOM/FrameElement/IFrameDisplayElement.h"
#include "DOM/FrameElement/IInstance.h"
#include "DOM/FrameElement/ISymbolInstance.h"
#include "DOM/FrameElement/IGroup.h"

// Timeline
#include "DOM/IFrame.h"

// Writer
#include "ResourcePublisher/Writer/Shared/SharedMovieclipWriter.h"

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
			std::u16string m_label = u"";

			// Frane symbols data in resource palette
			std::vector<std::tuple<uint16_t, FCM::BlendMode, std::u16string>> m_elementsData;

			// Transforms for each frame element
			std::vector<std::shared_ptr<MATRIX2D>> m_matrices;
			std::vector<std::shared_ptr<COLOR_MATRIX>> m_colors;

			// Tweeners
			FCM::AutoPtr<DOM::ITween> m_tween = nullptr;

			FCM::AutoPtr<IGeometricTweener> m_matrixTweener = nullptr;
			FCM::AutoPtr<IColorTweener> m_colorTweener = nullptr;
			FCM::AutoPtr<IShapeTweener> m_shapeTweener = nullptr;

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

		private:
			void AddFrameElementArray(FCM::FCMListPtr frameElements);
		};
	}
}