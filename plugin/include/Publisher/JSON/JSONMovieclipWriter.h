#pragma once

#include <string>
#include "Publisher/Shared/SharedMovieclipWriter.h"

#include "JSON.hpp"

using namespace nlohmann;

namespace sc {
	namespace Adobe {
		class JSONWriter;

		class JSONMovieclipWriter: public SharedMovieclipWriter {
			JSONWriter* m_writer = nullptr;

			ordered_json m_frames = json::array();

		public:
			void Init(JSONWriter* writer);

			void InitTimeline(uint32_t frameCount);

			void SetLabel(std::u16string label);

			void AddFrameElement(
				uint16_t id,
				FCM::BlendMode blending,
				std::u16string name,
				DOM::Utils::MATRIX2D* matrix,
				DOM::Utils::COLOR_MATRIX* color);

			void Finalize(uint16_t id, uint8_t fps);
		};
	}
}