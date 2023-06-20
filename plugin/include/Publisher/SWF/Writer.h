#pragma once

#include "Publisher/Shared/SharedWriter.h"

#include "Publisher/SWF/MovieclipWriter.h"
#include "Publisher/SWF/ShapeWriter.h"

#include "Module/AppContext.h"

#include "SupercellFlash.h"
#include "AtlasGenerator.h"
#include "opencv2/opencv.hpp"

using namespace FCM;

namespace sc {
	namespace Adobe {
		// Helper class for atlas generator
		struct Sprite {
			cv::Mat image;
			DOM::Utils::MATRIX2D matrix;
		};

		class Writer : public SharedWriter {
		public:
			using SharedWriter::SharedWriter;

			std::vector<Sprite> sprites;
			SupercellSWF swf;

			void Init() {};

			pSharedMovieclipWriter AddMovieclip();

			pSharedShapeWriter AddShape();

			void AddModifier(uint16_t id, sc::MovieClipModifier::Type type);

			void AddTextField(uint16_t id, TextFieldInfo field);

			void Finalize();
		};
	}
}