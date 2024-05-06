#pragma once

#include <algorithm>
#include <memory>

#include "ResourcePublisher/Writer/Shared/SharedWriter.h"
#include "SupercellFlash.h"

namespace sc {
	namespace Adobe {
		class SCWriter;

		class SCMovieclipWriter : public SharedMovieclipWriter {
			SCWriter& m_writer;
			SymbolContext& m_symbol;

			MovieClip m_object;

			SWFVector<std::optional<Matrix2D>> m_matrices;
			SWFVector<std::optional<ColorTransform>> m_colors;

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
			SCMovieclipWriter(SCWriter& writer, SymbolContext& symbol);
			virtual ~SCMovieclipWriter() = default;

			void InitializeTimeline(double fps, uint32_t frameCount);

			void SetLabel(const std::u16string& label);

			void AddFrameElement(
				uint16_t id,
				FCM::BlendMode blending,
				const std::u16string& name,
				std::optional<DOM::Utils::MATRIX2D> matrix,
				std::optional<DOM::Utils::COLOR_MATRIX> color
			);

			bool Finalize(uint16_t id);
		};
	}
}