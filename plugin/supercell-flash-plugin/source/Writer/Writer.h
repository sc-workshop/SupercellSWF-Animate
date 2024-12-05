#pragma once

#include "AnimateWriter.h"
#include "flash/flash.h"
#include "atlas_generator/Item/Item.h"
#include "core/memory/ref.h"

#include <filesystem>
#include <opencv2/opencv.hpp>

#include "GraphicItem/GraphicGroup.h"

namespace sc {
	namespace Adobe {
		class SCWriter : public Animate::Publisher::SharedWriter {
		public:
			SCWriter();
			virtual ~SCWriter();

		public:
			virtual Animate::Publisher::SharedMovieclipWriter* AddMovieclip(Animate::Publisher::SymbolContext& symbol);

			virtual Animate::Publisher::SharedShapeWriter* AddShape(Animate::Publisher::SymbolContext& symbol);

			virtual void AddModifier(uint16_t id, Animate::Publisher::MaskedLayerState type);

			virtual void AddTextField(uint16_t id, Animate::Publisher::SymbolContext& symbol, Animate::Publisher::TextElement& field);

			virtual void Finalize();

		public:
			// Path to temp sprite file
			const fs::path sprite_temp_path = fs::path(tmpnam(nullptr)).concat(".png");

			// kokoro
			flash::SupercellSWF swf;

		public:
			wk::Ref<cv::Mat> GetBitmap(const Animate::Publisher::SpriteElement& item);

			void AddGraphicGroup(const GraphicGroup& group);

		public:
			void LoadExternal();

			void FinalizeAtlas();

			// Some functions for atlas finalize

			void ProcessCommandTransform(
				flash::ShapeDrawBitmapCommand& command,
				wk::AtlasGenerator::Item::Transformation& transform,
				GraphicItem& item
			);

			void ProcessSpriteItem(
				flash::Shape& shape,
				wk::AtlasGenerator::Item& atlas_item,
				SpriteItem& sprite_item
			);

			void ProcessFilledItem(
				flash::Shape& shape,
				wk::AtlasGenerator::Item& atlas_item,
				FilledItem& filled_item
			);

			void ProcessSlicedItem(
				flash::Shape& shape,
				wk::AtlasGenerator::Item& atlas_item,
				SlicedItem& sprite_item
			);

		private:
			// Storage for Atlas Generator guys.
			// Represents swf shapes and must have the same size
			std::vector<GraphicGroup> m_graphic_groups;

			// Name / Image
			std::unordered_map<std::u16string, wk::Ref<cv::Mat>> m_cached_images;
		};
	}
}