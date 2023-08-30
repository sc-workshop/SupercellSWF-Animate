#pragma once

#include <memory>

#include "ResourcePublisher/Writer/Shared/SharedWriter.h"
#include "SupercellFlash.h"

namespace sc {
	namespace Adobe {
		class SCWriter;

		class SCMovieclipWriter : public SharedMovieclipWriter {
			SCWriter* m_writer = nullptr;
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
			SCMovieclipWriter(SCWriter* writer) : m_writer(writer) {};

			void Init(Context& context, SymbolContext& info, uint32_t frameCount);

			void SetLabel(std::u16string& label);

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