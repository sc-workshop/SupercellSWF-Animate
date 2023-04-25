#pragma once

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

			JSONNode* m_frames;

			Console console;

		public:
			JSONMovieclipWriter() {

				m_frames = new JSONNode(JSON_ARRAY);
				m_frames->set_name("frames");
			}
			~JSONMovieclipWriter() {
				delete m_frames;
			};

			Result Init(JSONWriter* writer, PIFCMCallback callback);

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