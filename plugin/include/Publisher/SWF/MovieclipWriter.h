#pragma once

#include "Publisher/Shared/SharedMovieclipWriter.h"
#include "io/Console.h"

using namespace FCM;

namespace sc {
	namespace Adobe {
		class Writer;

		class MovieclipWriter : public SharedMovieclipWriter {
			PIFCMCallback m_callback = nullptr;
			Writer* m_writer = nullptr;

			Console console;

		public:
			Result Init(Writer* writer, PIFCMCallback callback);

			Result InitTimeline(U_Int32 frameCount);

			Result SetLabel(U_Int32 frameIndex, std::string label);

			Result AddFrameElement(
				U_Int32 frameIndex,
				U_Int16 id,
				U_Int8 blending,
				std::string name,
				DOM::Utils::MATRIX2D* matrix,
				DOM::Utils::COLOR_MATRIX* color);

			void Finalize(U_Int16 id, U_Int8 fps, std::string name);
		};
	}
}