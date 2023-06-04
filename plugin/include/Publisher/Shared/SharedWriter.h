#pragma once

#include "SharedMovieclipWriter.h"
#include "SharedShapeWriter.h"
#include "Publisher/PublisherConfig.h"

#include <filesystem>
namespace fs = std::filesystem;

using namespace FCM;

namespace sc {
    namespace Adobe {
        class SharedWriter {
        public:
            virtual void Init(PIFCMCallback callback, const PublisherConfig& config) = 0;

            virtual pSharedMovieclipWriter AddMovieclip() = 0;
            virtual pSharedShapeWriter AddShape() = 0;

            virtual void AddModifier(uint16_t id, sc::MovieClipModifier::Type type) = 0;

            virtual void Finalize() = 0;
        };
    }
}