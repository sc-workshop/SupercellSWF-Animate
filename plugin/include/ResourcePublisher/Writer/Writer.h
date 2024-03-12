#pragma once

#include "ResourcePublisher/Writer/Shared/SharedWriter.h"

#include "AnimateSDK/app/DOM/FrameElement/ITextStyle.h"

#include "AtlasGenerator.h"
#include "SupercellFlash.h"

#include "ShapeWriter.h"
#include "MovieclipWriter.h"

#include "Module/PluginContext.h"
#include "Module/SessionConfig.h"
#include "Module/PluginException.h"

namespace sc {
	namespace Adobe {
		// Helper class for atlas generator
		struct Sprite {
			cv::Mat image;
			DOM::Utils::MATRIX2D matrix;

			// Filled Shape Contour
			std::vector<Point2D> contour;
		};

		struct ShapeSymbol
		{
			ShapeSymbol(SymbolContext& _symbol, const std::vector<Sprite>& _sprites) : symbol(_symbol), sprites(_sprites)
			{
			};

			SymbolContext& symbol;
			std::vector<Sprite> sprites;
		};

		class SCWriter : public SharedWriter {
		public:
			SCWriter();
			virtual ~SCWriter() = default;

			// Storage for atlas generator guys
			std::vector<ShapeSymbol> shapes;

			// kokoro
			SupercellSWF swf;

			virtual SharedMovieclipWriter* AddMovieclip(SymbolContext& symbol);

			virtual SharedShapeWriter* AddShape(SymbolContext& symbol);

			virtual void AddModifier(uint16_t id, MaskedLayerState type);

			virtual void AddTextField(uint16_t id, TextFieldInfo field);

			virtual void AddExportName(uint16_t id, std::string name);

			void LoadExternal();

			void FinalizeAtlas();

			virtual void Finalize();
		};
	}
}