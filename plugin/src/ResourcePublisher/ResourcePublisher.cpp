#include "ResourcePublisher/ResourcePublisher.h"

namespace sc {
	namespace Adobe {
		void ResourcePublisher::Publish(Context& context) {
			context.progressBar->window->SetProgress(
				(uint8_t)PublisherStage::LibraryProcessing
			);

			FCM::Double fps;
			FCM::U_Int8 framesPerSec;

			context.document->GetFrameRate(fps);
			framesPerSec = (FCM::U_Int8)fps;

			FCM::FCMListPtr libraryItems;
			context.document->GetLibraryItems(libraryItems.m_Ptr);

			shared_ptr<SharedWriter> writer;
			switch (context.config.method) {
			case PublisherMethod::SWF:
				writer = shared_ptr<SharedWriter>(new SCWriter());
				break;
			case PublisherMethod::JSON:
				writer = shared_ptr<SharedWriter>(new JSONWriter());
				break;
			default:
				throw exception("Failed to get writer");
			}

			writer->Init(context);

			ResourcePublisher resources(context, writer.get());
			resources.InitDocument(framesPerSec);

			std::vector<std::u16string> paths;
			GetItemsPaths(context, libraryItems, paths);

			StatusComponent* itemProgress = context.progressBar->window->GetAvailableProgressBar();
			ASSERT(itemProgress != nullptr);
			itemProgress->SetRange(paths.size());
			itemProgress->SetLabel(wxStringU16(context.locale.Get("TID_BAR_LABEL_LIBRARY_ITEMS")));

			for (uint32_t i = 0; paths.size() > i; i++) {
				std::u16string& path = paths[i];

				if (resources.context.progressBar->window->aboutToExit) {
					resources.context.close();
					return;
				}

				FCM::AutoPtr<DOM::ILibraryItem> item;
				context.document->GetLibraryItemByPath((FCM::CStringRep16)path.c_str(), item.m_Ptr);
				ASSERT(item != nullptr);

				itemProgress->SetStatus(wxStringU16(path));
				itemProgress->SetProgress(i);

				resources.AddLibraryItem(path, item);
			}

			resources.Finalize(paths);

			context.close();
		}

		void ResourcePublisher::GetItemsPaths(Context& context, FCM::FCMListPtr libraryItems, std::vector<std::u16string>& paths) {
			uint32_t itemCount = 0;
			libraryItems->Count(itemCount);

			uint32_t itemIndex = itemCount;
			for (uint32_t i = 0; i < itemCount; i++)
			{
				FCM::AutoPtr<DOM::ILibraryItem> item = libraryItems[--itemIndex];

				FCM::StringRep16 itemNamePtr;
				item->GetName(&itemNamePtr);
				std::u16string itemName = (const char16_t*)itemNamePtr;
				context.falloc->Free(itemNamePtr);

				FCM::AutoPtr<DOM::LibraryItem::IFolderItem> folderItem = item;
				if (folderItem)
				{
					FCM::FCMListPtr childrens;
					folderItem->GetChildren(childrens.m_Ptr);

					GetItemsPaths(context, childrens, paths);
				}
				else
				{
					FCM::AutoPtr<DOM::LibraryItem::ISymbolItem> symbolItem = item;
					if (!symbolItem) continue;

					FCM::AutoPtr<FCM::IFCMDictionary> dict;
					item->GetProperties(dict.m_Ptr);

					std::string symbolType;
					Utils::ReadString(dict, kLibProp_SymbolType_DictKey, symbolType);
					if (symbolType != "MovieClip") continue;

					paths.push_back(itemName);
				}
			}
		}

		uint16_t ResourcePublisher::AddLibraryItem(
			std::u16string name,
			FCM::AutoPtr<DOM::ILibraryItem> item
		) {
			FCM::AutoPtr<DOM::LibraryItem::ISymbolItem> symbolItem = item;
			FCM::AutoPtr<DOM::LibraryItem::IMediaItem> mediaItem = item;

			ASSERT(symbolItem != nullptr || mediaItem != nullptr);

			if (symbolItem) {
				SymbolContext symbol;
				symbol.name = name;

				FCM::AutoPtr<FCM::IFCMDictionary> properties;
				item->GetProperties(properties.m_Ptr);
				Utils::ReadString(properties, kLibProp_SymbolType_DictKey, symbol.type);

				return AddSymbol(name, symbolItem, symbol);
			}
			else if (mediaItem) {
				pSharedShapeWriter shape = m_writer->AddShape();
				cv::Mat image;
				graphicGenerator->GetImage(mediaItem, image);

				shape->AddGraphic(image, { 1, 0, 0, 1, 0, 0 });

				uint16_t identifer = m_id++;
				shape->Finalize(identifer);

				m_symbolsData[name] = identifer;

				return identifer;
			}

			throw std::exception("Unknown library item type");
		}

		uint16_t ResourcePublisher::AddSymbol(
			std::u16string name,
			DOM::LibraryItem::ISymbolItem* item,
			SymbolContext& symbol
		) {
			FCM::AutoPtr<DOM::ITimeline> timeline;
			item->GetTimeLine(timeline.m_Ptr);

			assert(timeline != nullptr);

			uint16_t result = UINT16_MAX;

			if (symbol.type != "MovieClip") {
				result = AddShape(name, timeline, symbol);
			}

			if (result == UINT16_MAX) {
				return AddMovieclip(name, timeline, symbol);
			}

			return result;
		};

		uint16_t ResourcePublisher::AddMovieclip(
			u16string name,
			FCM::AutoPtr<DOM::ITimeline1> timeline,
			SymbolContext& symbol
		) {
			CDocumentPage* page = timeline->GetDocPage();
			symbol.hasSlice9 = page->GetScale9();
			if (symbol.hasSlice9)
			{
				page->GetScale9Rect(symbol.slice9);
			}

			pSharedMovieclipWriter movieclip = m_writer->AddMovieclip();

			movieClipGenerator->Generate(context, movieclip, symbol, timeline);

			uint16_t identifer = m_id++;
			m_symbolsData[name] = identifer;

			movieclip->Finalize(identifer, m_fps);

			return identifer;
		};

		uint16_t ResourcePublisher::AddShape(
			u16string name,
			FCM::AutoPtr<DOM::ITimeline1> timeline,
			SymbolContext& symbol
		) {
			bool isShape = GraphicGenerator::Validate(timeline);

			if (!isShape) {
				return UINT16_MAX;
			}

			pSharedShapeWriter shape = m_writer->AddShape();

			graphicGenerator->Generate(context, symbol, shape, timeline);

			uint16_t identifer = m_id++;

			shape->Finalize(identifer);
			m_symbolsData[name] = identifer;

			return identifer;
		}

		uint16_t ResourcePublisher::AddModifier(
			sc::MovieClipModifier::Type type
		) {
			uint16_t identifer = m_id++;

			m_writer->AddModifier(identifer, type);
			m_modifierDict.push_back({ type, identifer });

			return identifer;
		}

		uint16_t ResourcePublisher::AddTextField(
			TextFieldInfo field
		) {
			uint16_t identifer = m_id++;

			m_writer->AddTextField(identifer, field);
			m_textfieldDict.push_back({ field, identifer });

			return identifer;
		}

		uint16_t ResourcePublisher::GetIdentifer(u16string name) {
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
			pSharedShapeWriter shape = m_writer->AddShape();

			uint16_t identifer = m_id++;

			shape->AddFilledShape(filledShape);

			shape->Finalize(identifer);
			m_filledShapeDict.push_back({ filledShape , identifer });

			return identifer;
		}

		uint16_t ResourcePublisher::GetIdentifer(
			sc::MovieClipModifier::Type type
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

		void ResourcePublisher::AddCachedBitmap(u16string name, cv::Mat image) {
			m_imagesData[name] = image;
		}

		bool ResourcePublisher::GetCachedBitmap(u16string name, cv::Mat& result) {
			auto it = m_imagesData.find(name);
			if (it != m_imagesData.end()) {
				result = it->second;
				return true;
			}
			else {
				return false;
			}
		}

		void ResourcePublisher::InitDocument(uint8_t fps) {
			m_fps = fps;

			m_symbolsData.clear();
			m_modifierDict.clear();
			m_imagesData.clear();
		}

		uint32_t& ResourcePublisher::GetSymbolUsage(std::u16string name) {
			auto it = m_symbolsUsage.find(name);

			if (it == m_symbolsUsage.end()) {
				m_symbolsUsage[name] = 0;
			}

			return m_symbolsUsage[name];
		}

		void ResourcePublisher::Finalize(std::vector<std::u16string> exports) {
			for (std::u16string pathStr : exports) {
				auto symbolDataIt = m_symbolsData.find(pathStr);

				if (symbolDataIt == m_symbolsData.end()) {
					continue;
				}

				if (context.config.exportsMode == ExportsMode::UnusedMovieclips) {
					auto usageDataIt = m_symbolsUsage.find(pathStr);
					if (usageDataIt != m_symbolsUsage.end()) {
						if (usageDataIt->second != 0) {
							continue;
						}
					}
				}

				fs::path name(pathStr);
				name = name.filename();

				m_writer->AddExportName(symbolDataIt->second, name.u16string());
			}
			m_writer->Finalize();
		}
	}
}