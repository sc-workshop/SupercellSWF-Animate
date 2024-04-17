#include "ResourcePublisher/ResourcePublisher.h"

namespace sc {
	namespace Adobe {
		void ResourcePublisher::Publish() {
			PluginContext& context = PluginContext::Instance();
			PluginSessionConfig& config = PluginSessionConfig::Instance();

			StatusComponent* publishStatus = context.Window()->CreateStatusBar(
				context.locale.GetString("TID_STATUS_INIT")
			);

			FCM::FCMListPtr libraryItems;
			config.document->GetLibraryItems(libraryItems.m_Ptr);

			std::vector<FCM::AutoPtr<DOM::ILibraryItem>> items;
			GetItems(libraryItems, items);

			publishStatus->SetRange(items.size());
			publishStatus->SetLabel(
				context.locale.GetString("TID_BAR_LABEL_LIBRARY_ITEMS")
			);

			for (size_t i = 0; items.size() > i; i++)
			{
				FCM::AutoPtr<DOM::ILibraryItem>& item = items[i];

				SymbolContext symbol(item);
				publishStatus->SetStatus(symbol.name);

				uint16_t id = AddLibraryItem(symbol, item);

				FCM::AutoPtr<FCM::IFCMDictionary> dict;
				item->GetProperties(dict.m_Ptr);

				std::string exportName;
				dict->Get(kLibProp_LinkageClass_DictKey, exportName);

				m_writer.AddExportName(id, exportName);

				if (context.Window()->aboutToExit) {
					context.DestroyWindow();
					return;
				}
				publishStatus->SetProgress(i + 1);
			}

			publishStatus->SetLabel(
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

					FCM::AutoPtr<FCM::IFCMDictionary> dict;
					FCM::Result status = item->GetProperties(dict.m_Ptr);

					if (FCM_FAILURE_CODE(status) || dict == nullptr) continue;

					FCM::U_Int32 valueLen;
					FCM::FCMDictRecTypeID type;

					status = dict->GetInfo(kLibProp_LinkageClass_DictKey, type, valueLen);
					if (FCM_FAILURE_CODE(status) || type != FCM::FCMDictRecTypeID::kFCMDictType_StringRep8)
					{
						continue;
					}

					result.push_back(item);
				}
			}
		}

		uint16_t ResourcePublisher::AddLibraryItem(
			SymbolContext& symbol,
			DOM::ILibraryItem* item
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
					shape_writer->Finalize(identifer);

					m_symbolsData[symbol.name] = identifer;

					delete shape_writer;
					return identifer;
				}
			}

			throw PluginException("TID_UNKNOWN_LIBRARY_ITEM_TYPE", symbol.name.c_str());
		}

		uint16_t ResourcePublisher::AddSymbol(
			SymbolContext& symbol,
			DOM::LibraryItem::ISymbolItem* item
		) {
			FCM::AutoPtr<DOM::ITimeline> timeline;
			item->GetTimeLine(timeline.m_Ptr);

			uint16_t result = UINT16_MAX;

			if (symbol.type != SymbolContext::SymbolType::MovieClip) {
				result = AddShape(symbol, timeline);
			}

			if (result == UINT16_MAX) {
				return AddMovieclip(symbol, timeline);
			}

			return result;
		};

		uint16_t ResourcePublisher::AddMovieclip(
			SymbolContext& symbol,
			FCM::AutoPtr<DOM::ITimeline1> timeline
		) {
			SharedMovieclipWriter* movieclip = m_writer.AddMovieclip(symbol);
			movieClipGenerator.Generate(*movieclip, symbol, timeline);

			uint16_t identifer = m_id++;
			m_symbolsData[symbol.name] = identifer;

			movieclip->Finalize(identifer);

			delete movieclip;

			return identifer;
		};

		uint16_t ResourcePublisher::AddShape(
			SymbolContext& symbol,
			FCM::AutoPtr <DOM::ITimeline1> timeline
		) {
			bool isShape = GraphicGenerator::Validate(timeline);

			if (!isShape) {
				return UINT16_MAX;
			}

			SharedShapeWriter* shape = m_writer.AddShape(symbol);

			graphicGenerator.Generate(symbol, *shape, timeline);

			uint16_t identifer = m_id++;

			shape->Finalize(identifer);
			m_symbolsData[symbol.name] = identifer;

			delete shape;

			return identifer;
		}

		uint16_t ResourcePublisher::AddModifier(
			MaskedLayerState type
		) {
			uint16_t identifer = m_id++;

			m_writer.AddModifier(identifer, type);
			m_modifierDict.push_back({ type, identifer });

			return identifer;
		}

		uint16_t ResourcePublisher::AddTextField(
			SymbolContext& symbol,
			TextElement& field
		) {
			uint16_t identifer = m_id++;

			m_writer.AddTextField(identifer, symbol, field);
			m_textfieldDict.push_back({ field, identifer });

			return identifer;
		}

		uint16_t ResourcePublisher::GetIdentifer(const std::u16string& name) {
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

			shape->Finalize(identifer);

			delete shape;

			return identifer;
		}

		uint16_t ResourcePublisher::GetIdentifer(
			MaskedLayerState type
		) {
			for (auto modifier : m_modifierDict) {
				if (modifier.first == type) {
					return modifier.second;
				}
			}

			return UINT16_MAX;
		}

		uint16_t ResourcePublisher::GetIdentifer(
			TextElement field
		) {
			for (auto textfield : m_textfieldDict) {
				if (textfield.first == field) {
					return textfield.second;
				}
			}

			return UINT16_MAX;
		}

		uint16_t ResourcePublisher::GetIdentifer(
			FilledElement shape
		) {
			for (auto shapePair : m_filledShapeDict) {
				if (shapePair.first == shape) {
					return shapePair.second;
				}
			}

			return UINT16_MAX;
		}
	}
}