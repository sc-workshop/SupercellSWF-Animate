#pragma once

#include "generic/ref.h"
#include "SharedMovieclipWriter.h"
#include "SharedGraphicWriter.h"
#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/TextElement.h"

#include <filesystem>
namespace fs = std::filesystem;

namespace sc {
	namespace Adobe {
		enum class MaskedLayerState : uint8_t
		{
			MASK_LAYER = 0,
			MASKED_LAYERS,
			MASKED_LAYERS_END
		};

		class SharedWriter {
		public:
			virtual ~SharedWriter() = default;

		public:
			virtual SharedMovieclipWriter* AddMovieclip(SymbolContext& symbol) = 0;

			virtual SharedShapeWriter* AddShape(SymbolContext& symbol) = 0;

			virtual void AddModifier(uint16_t id, MaskedLayerState type) = 0;

			virtual void AddTextField(uint16_t id, SymbolContext& symbol, TextElement& field) = 0;

			virtual void Finalize() = 0;
		};
	}
}