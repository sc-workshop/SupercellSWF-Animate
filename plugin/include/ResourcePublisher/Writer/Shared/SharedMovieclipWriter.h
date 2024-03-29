#pragma once

#include <vector>
#include <string>
#include <memory>

#include "AnimateSDK/core/common/FCMTypes.h"
#include "AnimateSDK/app/DOM/FrameElement/IFrameDisplayElement.h"
#include "Module/SymbolContext.h"

namespace sc {
	namespace Adobe {
		class SharedMovieclipWriter {
		public:
			virtual ~SharedMovieclipWriter() = default;

		protected:
			uint32_t m_position = 0;

		public:
			void next() {
				m_position++;
			}

			virtual void InitializeTimeline(double fps, uint32_t frameCount) = 0;

			virtual void SetLabel(const std::u16string& label) = 0;

			virtual void AddFrameElement(
				uint16_t id,
				FCM::BlendMode blending,
				const std::u16string& name,
				DOM::Utils::MATRIX2D* matrix,
				DOM::Utils::COLOR_MATRIX* color) = 0;

			virtual void Finalize(uint16_t id) = 0;
		};
	}
}