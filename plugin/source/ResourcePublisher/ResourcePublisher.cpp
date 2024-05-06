#include "ResourcePublisher/ResourcePublisher.h"

namespace sc {
	namespace Adobe {
		void ResourcePublisher::Publish() {
			PluginContext& context = PluginContext::Instance();
			PluginSessionConfig& config = PluginSessionConfig::Instance();

			StatusComponent* publishStatus = context.Window()->CreateStatusBarComponent(
				context.locale.GetString("TID_STATUS_INIT")
			);

			FCM::FCMListPtr libraryItems;
			config.document->GetLibraryItems(libraryItems.m_Ptr);

			std::vector<FCM::AutoPtr<DOM::ILibraryItem>> items;
			GetItems(libraryItems, items);

			publishStatus->SetRange(items.size());
			publishStatus->SetStatusLabel(
				context.locale.GetString("TID_BAR_LABEL_LIBRARY_ITEMS")
			);

			for (size_t i = 0; items.size() > i; i++)
			{
				FCM::AutoPtr<DOM::ILibraryItem>& item = items[i];

				FCM::AutoPtr<FCM::IFCMDictionary> dict;
				FCM::Result status = item->GetProperties(dict.m_Ptr);

				if (FCM_FAILURE_CODE(status) || dict == nullptr) continue;

				std::string linkage;
				dict->Get(kLibProp_LinkageClass_DictKey, linkage);

				if (linkage.empty()) continue;

				SymbolContext symbol(item, linkage);
				publishStatus->SetStatus(symbol.name);

				uint16_t id = AddLibraryItem(symbol, item);

				if (id == UINT16_MAX)
				{
					throw PluginException("TID_FAILED_TO_EXPORT_SYMBOL", symbol.name.c_str());
				}

				if (context.Window()->aboutToExit) {
					context.DestroyWindow();
					return;
				}
				publishStatus->SetProgress(i + 1);
			}

			publishStatus->SetStatusLabel(
				context.locale.GetString("TID_STATUS_SAVE")
			);
			publishStatus->SetStatus(u"");

			// Disable Close Button for writer finalizing process
			// TODO: make it possible to stop in finalize
			context.Window()->EnableCloseButton(false);

			m_writer.Finalize();

			context.Window()->DestroyStatusBar(publishStatus);
		}

		void ResourcePublisher::GetItems(FCM::FCMListPtr libraryItems, std::vector<FCM::AutoPtr<DOM::ILibraryItem>>& result) {
			uint32_t itemCount = 0;
			libraryItems->Count(itemCount);

			uint32_t itemIndex = itemCount;
			for (uint32_t i = 0; i < itemCount; i++)
			{
				FCM::AutoPtr<DOM::ILibraryItem> item = libraryItems[--itemIndex];

				FCM::AutoPtr<DOM::LibraryItem::IFolderItem> folderItem = item;
				if (folderItem)
				{
					FCM::FCMListPtr childrens;
					folderItem->GetChildren(childrens.m_Ptr);

					GetItems(childrens, result);
				}
				else
				{
					FCM::AutoPtr<DOM::LibraryItem::ISymbolItem> symbolItem = item;
					if (!symbolItem) continue;

					result.push_back(item);
				}
			}
		}

		uint16_t ResourcePublisher::AddLibraryItem(
			SymbolContext& symbol,
			FCM::AutoPtr<DOM::ILibraryItem> item
		) {
			FCM::AutoPtr<DOM::LibraryItem::ISymbolItem> symbol_item = item;
			FCM::AutoPtr<DOM::LibraryItem::IMediaItem> media_item = item;

			if (symbol_item) {
				return AddSymbol(symbol, symbol_item);
			}
			else if (media_item) {
				FCM::AutoPtr<FCM::IFCMUnknown> unknownMedia;
				media_item->GetMediaInfo(unknownMedia.m_Ptr);

				FCM::AutoPtr<DOM::MediaInfo::IBitmapInfo> bitmap = unknownMedia;

				if (bitmap)
				{
					SharedShapeWriter* shape_writer = m_writer.AddShape(symbol);

					SpriteElement element(item, media_item, bitmap);
					shape_writer->AddGraphic(element, { 1, 0, 0, 1, 0, 0 });

					uint16_t identifer = m_id++;
					bool sucess = shape_writer->Finalize(identifer);
					delete shape_writer;

					if (!sucess)
					{
						m_id--;
						return UINT16_MAX;
					}

					m_symbolsData[symbol.name] = identifer;

					return identifer;
				}
			}

			throw PluginException("TID_UNKNOWN_LIBRARY_ITEM_TYPE", symbol.name.c_str());
		}

		uint16_t ResourcePublisher::AddSymbol(
			SymbolContext& symbol,
			FCM::AutoPtr<DOM::LibraryItem::ISymbolItem> item
		) {
			FCM::AutoPtr<DOM::ITimeline> timeline;
			item->GetTimeLine(timeline.m_Ptr);

			if (symbol.type != SymbolContext::SymbolType::MovieClip && GraphicGenerator::Validate(timeline)) {
				return AddShape(symbol, timeline);
			}

			return AddMovieclip(symbol, timeline);
		};

		uint16_t ResourcePublisher::AddMovieclip(
			SymbolContext& symbol,
			FCM::AutoPtr<DOM::ITimeline1> timeline
		) {
			PluginContext& context = PluginContext::Instance();

			SharedMovieclipWriter* movieclip = m_writer.AddMovieclip(symbol);
			movieClipGenerator.Generate(*movieclip, symbol, timeline);

			uint16_t identifer = m_id++;
			bool sucess = movieclip->Finalize(identifer);
			delete movieclip;

			if (!sucess)
			{
				m_id--;
				return UINT16_MAX;
			}

			m_symbolsData[symbol.name] = identifer;

			context.logger->info("Added MovieClip: {}", Localization::ToUtf8(symbol.name));

			return identifer;
		};

		uint16_t ResourcePublisher::AddShape(
			SymbolContext& symbol,
			FCM::AutoPtr <DOM::ITimeline1> timeline
		) {
			PluginContext& context = PluginContext::Instance();

			SharedShapeWriter* shape = m_writer.AddShape(symbol);

			graphicGenerator.Generate(symbol, *shape, timeline);

			uint16_t identifer = m_id++;
			bool sucess = shape->Finalize(identifer);
			delete shape;

			if (!sucess)
			{
				m_id--;
				return UINT16_MAX;
			}

			m_symbolsData[symbol.name] = identifer;

			context.logger->info("Added Shape: {}", Localization::ToUtf8(symbol.name));

			return identifer;
		}

		uint16_t ResourcePublisher::AddModifier(
			MaskedLayerState type
		) {
			PluginContext& context = PluginContext::Instance();
			uint16_t identifer = m_id++;

			m_writer.AddModifier(identifer, type);
			m_modifierDict[type] = identifer;

			context.logger->info("Added Modifier: {}", (uint8_t)type);

			return identifer;
		}

		uint16_t ResourcePublisher::AddTextField(
			SymbolContext& symbol,
			TextElement& field
		) {
			PluginContext& context = PluginContext::Instance();

			uint16_t identifer = m_id++;

			m_writer.AddTextField(identifer, symbol, field);
			m_textfieldDict.push_back({ field ,identifer });

			context.logger->info("Added TextField from: {}", Localization::ToUtf8(symbol.name));

			return identifer;
		}

		uint16_t ResourcePublisher::GetIdentifer(const std::u16string& name) const {
			auto it = m_symbolsData.find(name);
			if (it != m_symbolsData.end()) {
				return it->second;
			}
			else {
				return UINT16_MAX;
			}
		}

		uint16_t ResourcePublisher::AddFilledElement(
			SymbolContext& symbol,
			const std::vector<FilledElement>& elements
		) {
			PluginContext& context = PluginContext::Instance();

			SymbolContext shape_symbol(symbol.name, SymbolContext::SymbolType::Graphic);
			SharedShapeWriter* shape = m_writer.AddShape(shape_symbol);

			uint16_t identifer = m_id++;

			if (symbol.slice_scaling.IsEnabled())
			{
				shape->AddSlicedElements(elements, symbol.slice_scaling.Guides());
			}
			else
			{
				for (const FilledElement& element : elements)
				{
					shape->AddFilledElement(element);
				}
			}

			bool sucess = shape->Finalize(identifer);
			delete shape;

			if (!sucess)
			{
				m_id--;
				return UINT16_MAX;
			}

			m_filledShapeDict.push_back({ elements, identifer });

			context.logger->info("Added FilledElement from: {}", Localization::ToUtf8(symbol.name));

			return identifer;
		}

		uint16_t ResourcePublisher::GetIdentifer(const MaskedLayerState& type) const {
			ModifierDict::const_iterator pos = m_modifierDict.find(type);
			if (pos != m_modifierDict.end())
			{
				return pos->second;
			}

			return UINT16_MAX;
		}

		uint16_t ResourcePublisher::GetIdentifer(const TextElement& field) const {
			for (const TextsDictValue& data : m_textfieldDict)
			{
				if (data.first == field)
				{
					return data.second;
				}
			}

			return UINT16_MAX;
		}

		uint16_t ResourcePublisher::GetIdentifer(const std::vector<FilledElement>& shape) const
		{
			for (const FilledDictValue& shapePair : m_filledShapeDict) {
				if (shapePair.first == shape) {
					return shapePair.second;
				}
			}

			return UINT16_MAX;
		}
	}
}