#pragma once

#include "ResourcePublisher/Writer/Shared/SharedWriter.h"

#include "DOM/FrameElement/ITextStyle.h"

#include "AtlasGenerator.h"
#include "SupercellFlash.h"

#include "ShapeWriter.h"
#include "MovieclipWriter.h"

namespace sc {
	namespace Adobe {
		// Helper class for atlas generator
		struct Sprite {
			cv::Mat image;
			DOM::Utils::MATRIX2D matrix;
			std::vector<Point2D> contour;
		};

		class SCWriter : public SharedWriter {
		private:
			Context* m_context = nullptr;

		public:
			std::vector<Sprite> sprites;
			SupercellSWF swf;

			void Init(Context& context);

			pSharedMovieclipWriter AddMovieclip();

			pSharedShapeWriter AddShape();

			void AddModifier(uint16_t id, sc::MovieClipModifier::Type type);

			void AddTextField(uint16_t id, TextFieldInfo field);

			void AddExportName(uint16_t id, std::u16string name);

			void LoadExternal();

			void Finalize();
		};
	}
}