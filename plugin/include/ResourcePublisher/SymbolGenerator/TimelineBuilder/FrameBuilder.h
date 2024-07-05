#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <tuple>
#include <optional>

// FCM stuff
#include "AnimateSDK/core/common/FCMTypes.h"
#include "AnimateSDK/app/DOM/Service/Tween/IPropertyIDs.h"

// MATRIX2D, COLOR_MATRIX
#include "AnimateSDK/app/DOM/Utils/DOMTypes.h"

// Tweeners stuff
#include "AnimateSDK/app/DOM/ITween.h"
#include "AnimateSDK/app/DOM/Service/Tween/ITweenerService.h"
#include "AnimateSDK/app/DOM/Service/Tween/IGeometricTweener.h"
#include "AnimateSDK/app/DOM/Service/Tween/IColorTweener.h"
#include "AnimateSDK/app/DOM/Service/Tween/IShapeTweener.h"

// Symbols
#include "AnimateSDK/app/DOM/ILibraryItem.h"
#include "AnimateSDK/app/DOM/FrameElement/IFrameDisplayElement.h"
#include "AnimateSDK/app/DOM/FrameElement/IInstance.h"
#include "AnimateSDK/app/DOM/FrameElement/ISymbolInstance.h"
#include "AnimateSDK/app/DOM/FrameElement/IGroup.h"

// Timeline
#include "AnimateSDK/app/DOM/IFrame.h"

// Writer
#include "ResourcePublisher/Writer/Shared/SharedMovieclipWriter.h"

// Fills
#include "FrameElements/FilledElement.h"

using DOM::Utils::MATRIX2D;
using DOM::Utils::COLOR_MATRIX;

namespace sc {
	namespace Adobe {
		class ResourcePublisher;

		class FrameBuilder {
		public:
			enum class LastElementType
			{
				None = 0,
				Symbol,
				TextField,
				FilledElement,
				SpriteElement
			};

		public:
			// A biiig workaround with magic numbers to get instance name from Graphic items
			static std::string GetInstanceName(FCM::AutoPtr<DOM::FrameElement::ISymbolInstance> symbol);

		private:
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
			std::vector<std::optional<MATRIX2D>> m_matrices;
			std::vector<std::optional<COLOR_MATRIX>> m_colors;

			// Tweeners
			FCM::AutoPtr<DOM::ITween> m_tween = nullptr;

			FCM::AutoPtr<DOM::Service::Tween::IGeometricTweener> m_matrixTweener = nullptr;
			FCM::AutoPtr<DOM::Service::Tween::IColorTweener> m_colorTweener = nullptr;
			FCM::AutoPtr<DOM::Service::Tween::IShapeTweener> m_shapeTweener = nullptr;

			std::vector<FilledElement> m_filled_elements;

			LastElementType m_last_element = LastElementType::None;

		public:
			FrameBuilder(ResourcePublisher& resources) : m_resources(resources) { };

			void Update(SymbolContext& symbol, FCM::AutoPtr<DOM::IFrame> frame);

			void releaseFrameElement(SymbolContext& symbol, SharedMovieclipWriter& writer, size_t index);

			void operator()(SymbolContext& symbol, SharedMovieclipWriter& writer);

			bool flushMask() const
			{
				return !m_elementsData.empty() || !m_filled_elements.empty();
			}

			void next()
			{
				m_position++;
			}

			uint32_t duration() const
			{
				return m_duration;
			}

			uint32_t position() const
			{
				return m_position;
			}

			LastElementType last_element() const
			{
				return m_last_element;
			}

			operator bool() const
			{
				return m_duration > m_position;
			}

			const std::vector<FilledElement>& filledElements() const
			{
				return m_filled_elements;
			}

			void ApplyName(SharedMovieclipWriter& writer) const
			{
				if (!m_label.empty()) {
					writer.SetLabel(m_label);
				}
			}

			void releaseFilledElements(SymbolContext& symbol, std::u16string& name);
			void inheritFilledElements(const FrameBuilder& frame);

		private:
			void DeclareFrameElements(
				SymbolContext& symbol,
				FCM::FCMListPtr frameElements,
				std::optional<MATRIX2D> base_transform = std::nullopt,
				bool reverse = false
			);

			void DeclareFrameElement(
				SymbolContext& symbol,
				FCM::AutoPtr<DOM::FrameElement::IFrameDisplayElement> frameElement,
				std::optional<MATRIX2D> base_transform = std::nullopt
			);
		};
	}
}