#pragma once

#include "AnimateWriter.h"
#include "flash/flash.h"
#include "atlas_generator/Item/Item.h"
#include "core/memory/ref.h"
#include "Module/Module.h"

#include <filesystem>

#include "Writer/GraphicItem/GraphicItem.h"
#include "Writer/GraphicItem/FilledItem.h"
#include "Writer/GraphicItem/SlicedItem.h"
#include "Writer/GraphicItem/SpriteItem.h"

namespace sc {
	namespace Adobe {
		using namespace Animate::Publisher;

		class SCWriter : public SharedWriter {
		public:
			using GraphicGroup = StaticElementsGroup;

		public:
			SCWriter();
			virtual ~SCWriter();

		public:
			virtual void SetExportedSymbols(const std::vector<SymbolContext>& symbols) override;
			virtual wk::Ref<SharedMovieclipWriter> AddMovieclip(Animate::Publisher::SymbolContext& symbol);
			virtual wk::Ref<SharedShapeWriter> AddShape(Animate::Publisher::SymbolContext& symbol);
			virtual wk::Ref<SharedTextFieldWriter> AddTextField(Animate::Publisher::SymbolContext& symbol);

			virtual void AddModifier(uint16_t id, Animate::Publisher::MaskedLayerState type);

			virtual void Finalize();

		public:
			// Path to temp sprite file
			const fs::path sprite_temp_path = fs::path(tmpnam(nullptr)).concat(".png");

			// kokoro
			flash::SupercellSWF swf;
			size_t texture_offset = 0;

		public:
			void IncrementSymbolsProcessed();
			wk::RawImageRef GetBitmap(const Animate::Publisher::BitmapElement& item);
			void AddGraphicGroup(const GraphicGroup& group);

		public:
			uint16_t LoadExternal(fs::path path);

			void FinalizeAtlas();
			void SetupTextureSettings();

			// Some functions for atlas finalize
			// TODO: move to separate class
			void ProcessDrawCommand(
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
			std::unordered_map<std::u16string, wk::RawImageRef> m_cached_images;

			// Total symbols status bar
			size_t m_symbols_processed = 0;
			StatusComponent* m_status = 0;
		};
	}
}