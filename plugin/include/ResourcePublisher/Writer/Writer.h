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

			virtual void AddTextField(uint16_t id, TextElement field);

			virtual void AddExportName(uint16_t id, std::string name);

			virtual void Finalize();

		public:
			// Path to temp file for sprites
			const fs::path sprite_temp_path = fs::path(tmpnam(nullptr)).concat(".png");

			// kokoro
			SupercellSWF swf;

		public:
			Ref<cv::Mat> GetBitmap(const SpriteElement& item)
			{
				std::u16string name = item.name();

				if (m_cached_images.count(name))
				{
					return m_cached_images[name];
				}

				item.exportImage(sprite_temp_path);

				Ref<cv::Mat> image = CreateRef<cv::Mat>(cv::imread(sprite_temp_path.string(), cv::IMREAD_UNCHANGED));
				m_cached_images[name] = image;

				return image;
			}

			void AddGraphicGroup(const GraphicGroup& group)
			{
				m_graphic_groups.push_back(group);
			}

		public:
			void LoadExternal();

			void FinalizeAtlas();

			// Some functions for atlas finalize

			void TransformCommand(
				ShapeDrawBitmapCommand& command,
				Matrix2x3<float> matrix,
				AtlasGenerator::Item::Transformation& transform
			);

			void ProcessSpriteItem(
				Shape& shape,
				AtlasGenerator::Item& atlas_item,
				SpriteItem& sprite_item
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