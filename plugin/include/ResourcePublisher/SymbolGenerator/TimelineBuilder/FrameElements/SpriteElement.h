#pragma once

#include <filesystem>

#include "AnimateSDK/app/DOM/ILibraryItem.h"
#include "AnimateSDK/app/DOM/MediaInfo/IBitmapInfo.h"
#include "AnimateSDK/app/DOM/Service/Image/IBitmapExportService.h"

#include "Module/PluginContext.h"

namespace sc
{
	namespace Adobe
	{
		class SpriteElement
		{
		public:
			SpriteElement(DOM::ILibraryItem* library_item, DOM::LibraryItem::IMediaItem* media_item, DOM::MediaInfo::IBitmapInfo* bitmap);
			~SpriteElement();

		public:
			const char16_t* name() const;
			void exportImage(std::filesystem::path path) const;

		private:
			DOM::ILibraryItem* m_item;
			DOM::LibraryItem::IMediaItem* m_media_item;
			DOM::MediaInfo::IBitmapInfo* m_info;

			char16_t* m_name;
		};
	}
}