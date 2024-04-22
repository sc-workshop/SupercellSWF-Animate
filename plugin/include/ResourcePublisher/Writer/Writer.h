#pragma once

#include <unordered_map>

#include "ResourcePublisher/Writer/Shared/SharedWriter.h"
#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/SpriteElement.h"

#include "AnimateSDK/app/DOM/FrameElement/ITextStyle.h"

#include "AtlasGenerator/Generator.h"
#include "GraphicItem/GraphicGroup.h"
#include "SupercellFlash.h"

#include "ShapeWriter.h"
#include "MovieclipWriter.h"

#include "Module/PluginContext.h"
#include "Module/SessionConfig.h"
#include "Module/PluginException.h"
#include "Module/Localization.h"

namespace sc {
	namespace Adobe {
		class SCWriter : public SharedWriter {
		public:
			SCWriter();
			virtual ~SCWriter();

		public:
			virtual SharedMovieclipWriter* AddMovieclip(SymbolContext& symbol);

			virtual SharedShapeWriter* AddShape(SymbolContext& symbol);

			virtual void AddModifier(uint16_t id, MaskedLayerState type);

			virtual void AddTextField(uint16_t id, SymbolContext& symbol, TextElement& field);

			virtual void AddExportName(uint16_t id, const std::string& name);

			virtual void Finalize();

		public:
			// Path to temp sprite file
			const fs::path sprite_temp_path = fs::path(tmpnam(nullptr)).concat(".png");

			// kokoro
			SupercellSWF swf;

		public:
			Ref<cv::Mat> GetBitmap(const SpriteElement& item);

			void AddGraphicGroup(const GraphicGroup& group);

		public:
			void LoadExternal();

			void FinalizeAtlas();

			// Some functions for atlas finalize

			void ProcessCommandTransform(
				ShapeDrawBitmapCommand& command,
				AtlasGenerator::Item::Transformation& transform,
				GraphicItem& item
			);

			void ProcessSpriteItem(
				Shape& shape,
				AtlasGenerator::Item& atlas_item,
				SpriteItem& sprite_item
			);

			void ProcessFilledItem(
				Shape& shape,
				AtlasGenerator::Item& atlas_item,
				FilledItem& filled_item
			);

			void ProcessSlicedItem(
				Shape& shape,
				AtlasGenerator::Item& atlas_item,
				SlicedItem& sprite_item
			);

		private:
			// Storage for Atlas Generator guys.
			// Represents swf shapes and must have the same size
			std::vector<GraphicGroup> m_graphic_groups;

			// Name / Image
			std::unordered_map<std::u16string, Ref<cv::Mat>> m_cached_images;
		};
	}
}