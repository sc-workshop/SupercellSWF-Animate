#pragma once

#include "SharedMovieclipWriter.h"
#include "SharedShapeWriter.h"

#include <filesystem>
namespace fs = std::filesystem;

using namespace FCM;

namespace sc {
    namespace Adobe {
        class SharedWriter {
        public:
            fs::path baseFolder = fs::current_path();

        public:
            virtual SharedMovieclipWriter* _FCMCALL AddMovieclip() = 0;
            virtual SharedShapeWriter* _FCMCALL AddShape() = 0;

            virtual void Finalize(std::string outputPath) = 0;
        };
    }
}