#include "ResourcePublisher/ResourcePublisher.h"

namespace sc {
	namespace Adobe {
		void ResourcePublisher::Publish() {
			//FCM::Double fps;
			//FCM::U_Int8 framesPerSec;
			//
			//context.document->GetFrameRate(fps);
			//framesPerSec = (FCM::U_Int8)fps;
			PluginContext& context = PluginContext::Instance();
			PluginSessionConfig& config = PluginSessionConfig::Instance();

			StatusComponent* publishStatus = context.window()->CreateStatusBar(
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

				if (context.window()->aboutToExit) {
					context.destroyWindow();
					return;
				}
				publishStatus->SetProgress(i+1);
			}

			publishStatus->SetLabel(
				context.locale.GetString("TID_STATUS_SAVE")
			);
			publishStatus->SetStatus(u"");

			// Disable Close Button for writer finalizing process
			context.window()->EnableCloseButton(false);

			m_writer.Finalize();

			publishStatus->Destroy();
		}

		void ResourcePublisher::GetItems(FCM::FCMListPtr libraryItems, std::vector<FCM::AutoPtr<DOM::ILibraryItem>>& result) {
			PluginContext& context = PluginContext::Instance();

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
					item->GetProperties(dict.m_Ptr);

					FCM::U_Int32 valueLen;
					FCM::FCMDictRecTypeID type;

					FCM::Result status = dict->GetInfo(kLibProp_LinkageClass_DictKey, type, valueLen);
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
			FCM::AutoPtr<DOM::LibraryItem::ISymbolItem> symbolItem = item;
			FCM::AutoPtr<DOM::LibraryItem::IMediaItem> mediaItem = item;

			if (symbolItem) {
				return AddSymbol(symbol, symbolItem);
			}
			else if (mediaItem) {
				SharedShapeWriter* shape = m_writer.AddShape(symbol);

				cv::Mat image;
				graphicGenerator.GetImage(mediaItem, image);

				shape->AddGraphic(image, { 1, 0, 0, 1, 0, 0 });

				uint16_t identifer = m_id++;
				shape->Finalize(identifer);

				m_symbolsData[symbol.name] = identifer;

				delete shape;
				return identifer;
			}

			throw GeneralRuntimeException("Unknown Library Item Type");
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
			CDocumentPage* page = timeline->GetDocPage();
			//symbol.hasSlice9 = page->GetScale9();
			//if (symbol.hasSlice9)
			//{
			//	page->GetScale9Rect(symbol.slice9);
			//}

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
			TextFieldInfo field
		) {
			uint16_t identifer = m_id++;

			m_writer.AddTextField(identifer, field);
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

		uint16_t ResourcePublisher::AddFilledShape(
			FilledShape filledShape
		) {
			SymbolContext symbol(u"", SymbolContext::SymbolType::Graphic);
			SharedShapeWriter* shape = m_writer.AddShape(symbol);

			uint16_t identifer = m_id++;

			shape->AddFilledShape(filledShape);

			shape->Finalize(identifer);
			m_filledShapeDict.push_back({ filledShape , identifer });

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
			TextFieldInfo field
		) {
			for (auto textfield : m_textfieldDict) {
				if (textfield.first == field) {
					return textfield.second;
				}
			}

			return UINT16_MAX;
		}

		uint16_t ResourcePublisher::GetIdentifer(
			FilledShape shape
		) {
			for (auto shapePair : m_filledShapeDict) {
				if (shapePair.first == shape) {
					return shapePair.second;
				}
			}

			return UINT16_MAX;
		}

		void ResourcePublisher::AddCachedBitmap(const std::u16string& name, cv::Mat image) {
			m_imagesData[name] = image;
		}

		bool ResourcePublisher::GetCachedBitmap(const std::u16string& name, cv::Mat& result) {
			auto it = m_imagesData.find(name);
			if (it != m_imagesData.end()) {
				result = it->second;
				return true;
			}
			else {
				return false;
			}
		}
	}
}