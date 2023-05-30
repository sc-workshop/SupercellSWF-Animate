#pragma once

#include "Publisher/Shared/SharedMovieclipWriter.h"
#include "io/Console.h"

#include <memory>
#include <SupercellFlash.h>

using namespace FCM;

namespace sc {
	namespace Adobe {
		class Writer;

		class MovieclipWriter : public SharedMovieclipWriter {
			PIFCMCallback m_callback = nullptr;
			Writer* m_writer = nullptr;
			pMovieClip m_object = pMovieClip(new MovieClip());

			std::vector<pMatrix2D> m_matrices;
			std::vector<pColorTransform> m_colors;

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
				DOM::Utils::MATRIX2D& matrix,
				DOM::Utils::COLOR_MATRIX& color);

			void Finalize(U_Int16 id, U_Int8 fps, std::string name);

			// Helper functions

			uint16_t GetInstanceIndex(
				U_Int32 elementsOffset,
				U_Int16 elementsCount,
				U_Int16 id,
				U_Int8 blending,
				std::string name
			);

			void FinalizeTransforms();

			bool FinalizeElementsTransform(uint8_t& bankIndex);
		};
	}
}