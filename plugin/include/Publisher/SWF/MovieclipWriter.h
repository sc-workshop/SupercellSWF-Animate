#pragma once

#include "Publisher/Shared/SharedMovieclipWriter.h"
#include "Module/AppContext.h"

#include <memory>
#include <SupercellFlash.h>

namespace sc {
	namespace Adobe {
		class Writer;

		class MovieclipWriter : public SharedMovieclipWriter {
			Writer* m_writer = nullptr;
			pMovieClip m_object = pMovieClip(new MovieClip());

			std::vector<pMatrix2D> m_matrices;
			std::vector<pColorTransform> m_colors;

			// Helper functions

			uint16_t GetInstanceIndex(
				uint16_t elementsCount,
				uint16_t id,
				FCM::BlendMode blending,
				std::string name
			);

			void FinalizeTransforms();

			bool FinalizeElementsTransform(uint8_t& bankIndex);

		public:
			void Init(Writer* writer);

			void InitTimeline(uint32_t frameCount);

			void SetLabel(u16string label);

			void AddFrameElement(
				uint16_t id,
				FCM::BlendMode blending,
				std::u16string name,
				DOM::Utils::MATRIX2D* matrix,
				DOM::Utils::COLOR_MATRIX* color
			);

			void Finalize(uint16_t id, uint8_t fps);
		};
	}
}