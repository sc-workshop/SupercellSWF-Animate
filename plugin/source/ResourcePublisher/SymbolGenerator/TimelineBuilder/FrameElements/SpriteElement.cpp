#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/SpriteElement.h"

namespace sc
{
	namespace Adobe
	{
		SpriteElement::SpriteElement(DOM::ILibraryItem* library_item, DOM::LibraryItem::IMediaItem* media_item, DOM::MediaInfo::IBitmapInfo* bitmap) :
			m_item(library_item), m_media_item(media_item), m_info(bitmap)
		{
			library_item->GetName((FCM::StringRep16*)&m_name);
		}

		SpriteElement::~SpriteElement()
		{
			PluginContext& context = PluginContext::Instance();

			if (m_name)
			{
				context.falloc->Free(m_name);
			}
		}

		const char16_t* SpriteElement::name() const
		{
			return (const char16_t*)m_name;
		}

		void SpriteElement::exportImage(std::filesystem::path path) const
		{
			PluginContext& context = PluginContext::Instance();

			auto service = context.getService<DOM::Service::Image::IBitmapExportService>(DOM::FLA_BITMAP_SERVICE);

			FCM::Result res = service->ExportToFile(
				m_media_item,
				(FCM::CStringRep16)path.u16string().c_str(),
				100
			);
			if (FCM_FAILURE_CODE(res)) {
				throw PluginException("TID_BITMAP_SERVICE_EXPORT_FAILED");
			}
		}
	}
}