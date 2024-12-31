#pragma once

#include "AnimateWriter.h"
#include "flash/flash.h"
#include "atlas_generator/Item/Item.h"
#include "core/memory/ref.h"

#include <filesystem>
#include <opencv2/opencv.hpp>

#include "Writer/GraphicItem/GraphicItem.h"
#include "Writer/GraphicItem/FilledItem.h"
#include "Writer/GraphicItem/SlicedItem.h"
#include "Writer/GraphicItem/SpriteItem.h"

namespace sc {
	namespace Adobe {
		class SCWriter : public Animate::Publisher::SharedWriter {
		public:
			using GraphicGroup = Animate::Publisher::StaticElementsGroup;

		public:
			SCWriter();
			virtual ~SCWriter();

		public:
			virtual Animate::Publisher::SharedMovieclipWriter* AddMovieclip(Animate::Publisher::SymbolContext& symbol);
			virtual Animate::Publisher::SharedShapeWriter* AddShape(Animate::Publisher::SymbolContext& symbol);
			virtual Animate::Publisher::SharedTextFieldWriter* AddTextField(Animate::Publisher::SymbolContext& symbol);

			virtual void AddModifier(uint16_t id, Animate::Publisher::MaskedLayerState type);

			virtual void Finalize();

		public:
			// Path to temp sprite file
			const fs::path sprite_temp_path = fs::path(tmpnam(nullptr)).concat(".png");

			// kokoro
			flash::SupercellSWF swf;

		public:
			wk::Ref<cv::Mat> GetBitmap(const Animate::Publisher::BitmapElement& item);

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

			void ProcessVertices(
				flash::Shape& shape,
				const wk::AtlasGenerator::Container<wk::AtlasGenerator::Vertex>& vertices,
				wk::AtlasGenerator::Item& atlas_item,
				GraphicItem& item
			);

			void ProcessSpriteItem(
				flash::Shape& shape,
				wk::AtlasGenerator::Item& atlas_item,
				BitmapItem& sprite_item
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
			// Represents swf shapes and must have the same size as shapes vector
			std::vector<GraphicGroup> m_graphic_groups;

			// Name / Image
			std::unordered_map<std::u16string, wk::Ref<cv::Mat>> m_cached_images;
		};
	}
}