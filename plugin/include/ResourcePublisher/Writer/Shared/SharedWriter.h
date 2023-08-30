#pragma once

#include "SharedMovieclipWriter.h"
#include "SharedGraphicWriter.h"
#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/TextField.h"

#include "SupercellFlash/objects/MovieClipModifier.h"

#include <filesystem>
namespace fs = std::filesystem;

namespace sc {
	namespace Adobe {
		class Context;

		class SharedWriter {
		public:
			virtual void Init(Context& context) = 0;

			virtual pSharedMovieclipWriter AddMovieclip() = 0;

			virtual pSharedShapeWriter AddShape() = 0;

			virtual void AddModifier(uint16_t id, sc::MovieClipModifier::Type type) = 0;

			virtual void AddTextField(uint16_t id, TextFieldInfo field) = 0;

			virtual void AddExportName(uint16_t id, std::u16string name) = 0;

			virtual void Finalize() = 0;
		};
	}
}