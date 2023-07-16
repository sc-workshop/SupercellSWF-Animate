#pragma once

#include "SharedMovieclipWriter.h"
#include "SharedShapeWriter.h"
#include "Module/AppContext.h"
#include "Publisher/TimelineBuilder/FrameElements/TextField.h"

#include <filesystem>
namespace fs = std::filesystem;

namespace sc {
    namespace Adobe {
        class SharedWriter {
        protected:
            AppContext& m_context;

        public:
            SharedWriter(AppContext& context): m_context(context) {}

            virtual void Init() = 0;

            virtual pSharedMovieclipWriter AddMovieclip() = 0;

            virtual pSharedShapeWriter AddShape() = 0;

            virtual void AddModifier(uint16_t id, sc::MovieClipModifier::Type type) = 0;

            virtual void AddTextField(uint16_t id, TextFieldInfo field) = 0;

            virtual void AddExportName(uint16_t id, std::u16string name) = 0;

            virtual void Finalize() = 0;
        };
    }
}