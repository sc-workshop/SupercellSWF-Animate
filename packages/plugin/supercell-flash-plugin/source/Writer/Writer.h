#pragma once

#include "AnimateWriter.h"
#include "flash/flash.h"
#include "core/memory/ref.h"
#include "Module/Module.h"
#include "AtlasWriter.h"

#include <filesystem>

namespace sc::Adobe {
	using namespace Animate::Publisher;

	class SCWriter : public SharedWriter {
	public:
		friend AtlasWriter;
		using GraphicGroup = StaticElementsGroup;

	public:
		SCWriter();
		virtual ~SCWriter();

	public:
		virtual void SetExportedSymbols(const std::vector<SymbolContext>& symbols) override;
		virtual wk::Ref<SharedMovieclipWriter> AddMovieclip(SymbolContext& symbol);
		virtual wk::Ref<SharedShapeWriter> AddShape(SymbolContext& symbol);
		virtual wk::Ref<SharedTextFieldWriter> AddTextField(SymbolContext& symbol);

		virtual void AddModifier(uint16_t id, MaskedLayerState type);

		virtual void Finalize();

	public:
		// Path to temp sprite file
		const fs::path sprite_temp_path = fs::path(tmpnam(nullptr)).concat(".png");

		// kokoro
		flash::SupercellSWF swf;

	public:
		void IncrementSymbolsProcessed();
		wk::RawImageRef GetBitmap(const BitmapElement& item);
		void AddGraphicGroup(const GraphicGroup& group);

	public:
		uint16_t LoadExternal(fs::path path);
		void SetupTextureSettings();

	private:
		// Storage for Atlas Generator guys.
		// Represents swf shapes and must have the same size as shapes vector
		std::vector<GraphicGroup> m_graphic_groups;

		// Name / Image
		std::unordered_map<std::u16string, wk::RawImageRef> m_cached_images;

		// Total symbols status bar
		size_t m_symbols_processed = 0;
		StatusComponent* m_status = 0;
		
		AtlasWriter m_atlas;
	};
}