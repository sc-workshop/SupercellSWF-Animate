#pragma once

#include "Publisher/Shared/SharedWriter.h"

#include "Publisher/SWF/MovieclipWriter.h"
#include "Publisher/SWF/ShapeWriter.h"

#include "io/Console.h"

#include "SupercellFlash.h"
#include "AtlasGenerator.h"
#include "opencv2/opencv.hpp"

#include <locale> 
#include <codecvt> 

using namespace FCM;

namespace sc {
	namespace Adobe {
		// Helper class for atlas generator
		struct Sprite {
			cv::Mat image;
			DOM::Utils::MATRIX2D matrix;
		};

		class Writer : public SharedWriter {
			Console console;
			PIFCMCallback m_callback = nullptr;
			PublisherConfig m_config;

		public:
			std::vector<Sprite> sprites;

			SupercellSWF swf;

			void Init(PIFCMCallback callback, const PublisherConfig& config);

			pSharedMovieclipWriter AddMovieclip();
			pSharedShapeWriter AddShape();

			void AddModifier(uint16_t id, sc::MovieClipModifier::Type type) {
				pMovieClipModifier modifier = pMovieClipModifier(new MovieClipModifier());
				modifier->id(id);
				modifier->type(type);
				swf.movieClipModifiers.push_back(modifier);
			}

			void FinalizeAtlas();

			void Finalize();
		};
	}
}