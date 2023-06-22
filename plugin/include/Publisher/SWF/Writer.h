#pragma once

#include "Publisher/Shared/SharedWriter.h"

#include "Publisher/SWF/MovieclipWriter.h"
#include "Publisher/SWF/ShapeWriter.h"

#include "Module/AppContext.h"

#include "SupercellFlash.h"
#include "AtlasGenerator.h"
#include "opencv2/opencv.hpp"


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
			SupercellSWF m_swf;

			void Init() {
				if (m_context.config.exportToExternal) {
					if (m_context.config.exportToExternalPath.empty()) {
						throw exception("Failed to get external file path");
					}
				}
			};

			pSharedMovieclipWriter AddMovieclip();

			pSharedShapeWriter AddShape();

			void AddModifier(uint16_t id, sc::MovieClipModifier::Type type);

			void AddTextField(uint16_t id, TextFieldInfo field);

			void LoadExternal();

			void Finalize();
		};
	}
}