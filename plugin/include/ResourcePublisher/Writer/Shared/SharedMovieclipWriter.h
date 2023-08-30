#pragma once

#include <vector>
#include <string>
#include <memory>

#include "FCMTypes.h"
#include "DOM/FrameElement/IFrameDisplayElement.h"
#include "ResourcePublisher/SymbolGenerator/SymbolBehavior.h"

namespace sc {
	namespace Adobe {
		class Context;

		class SharedMovieclipWriter {
		protected:
			uint32_t m_position = 0;

		public:
			void next() {
				m_position++;
			}

			virtual void Init(Context& context, SymbolBehaviorInfo& info, uint32_t frameCount) = 0;

			virtual void SetLabel(std::u16string& label) = 0;

			virtual void AddFrameElement(
				uint16_t id,
				FCM::BlendMode blending,
				std::u16string name,
				DOM::Utils::MATRIX2D* matrix,
				DOM::Utils::COLOR_MATRIX* color) = 0;

			virtual void Finalize(uint16_t id, uint8_t fps) = 0;
		};

		typedef std::shared_ptr<SharedMovieclipWriter> pSharedMovieclipWriter;
	}
}