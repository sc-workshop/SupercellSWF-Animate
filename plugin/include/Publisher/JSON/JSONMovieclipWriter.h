#pragma once

#include <string>
#include "Publisher/Shared/SharedMovieclipWriter.h"
#include "io/Console.h"

#include "libjson.h"

using namespace FCM;

namespace sc {
	namespace Adobe {
		class JSONWriter;

		class JSONMovieclipWriter: public SharedMovieclipWriter {
			PIFCMCallback m_callback = nullptr;
			JSONWriter* m_writer = nullptr;

			JSONNode m_frames = JSONNode(JSON_ARRAY);

			Console console;

		public:
			void Init(JSONWriter* writer, PIFCMCallback callback);

			void InitTimeline(uint32_t frameCount);

			void SetLabel(std::string label);

			void AddFrameElement(
				uint16_t id,
				uint8_t blending,
				std::string name,
				DOM::Utils::MATRIX2D* matrix,
				DOM::Utils::COLOR_MATRIX* color);

			void Finalize(uint16_t id, uint8_t fps, std::u16string name);
		};
	}
}