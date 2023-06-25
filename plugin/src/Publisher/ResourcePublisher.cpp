#include "Publisher/ResourcePublisher.h"

namespace sc {
	namespace Adobe {

        void ResourcePublisher::Publish(AppContext& context) {
            context.window->ui->SetProgress(
                (uint8_t)PublisherExportStage::LibraryProcessing
            );
            context.window->ui->SetStatus("");

            FCM::Double fps;
            FCM::U_Int8 framesPerSec;

            context.document->GetFrameRate(fps);
            framesPerSec = (FCM::U_Int8)fps;

            FCM::FCMListPtr libraryItems;
            context.document->GetLibraryItems(libraryItems.m_Ptr);

            shared_ptr<SharedWriter> writer;
            switch (context.config.method) {
            case PublisherMethod::SWF:
                writer = shared_ptr<SharedWriter>(new Writer(context));
                break;
            case PublisherMethod::JSON:
                writer = shared_ptr<SharedWriter>(new JSONWriter(context));
                break;
            default:
                throw exception("Failed to get writer");
            }

            writer->Init();

            ResourcePublisher resources(context, writer.get());
            resources.InitDocument(framesPerSec);

            ResourcePublisher::PublishItems(libraryItems, resources);

            resources.Finalize();

            context.close();
        }

        void ResourcePublisher::PublishItems(FCM::FCMListPtr libraryItems, ResourcePublisher& resources) {
            uint32_t itemCount = 0;
            libraryItems->Count(itemCount);

            for (uint32_t i = 0; i < itemCount; i++)
            {
                FCM::AutoPtr<DOM::ILibraryItem> item = libraryItems[i];

                FCM::StringRep16 itemNamePtr;
                item->GetName(&itemNamePtr);
                u16string itemName = (const char16_t*)itemNamePtr;
                resources.context.falloc->Free(itemNamePtr);

                FCM::AutoPtr<DOM::LibraryItem::IFolderItem> folderItem = item;
                if (folderItem)
                {
                    FCM::FCMListPtr childrens;
                    folderItem->GetChildren(childrens.m_Ptr);

                    // Export all its children
                    ResourcePublisher::PublishItems(childrens, resources);
                }
                else
                {
                    FCM::AutoPtr<DOM::LibraryItem::ISymbolItem> symbolItem = item;
                    if (!symbolItem) continue;

                    FCM::AutoPtr<FCM::IFCMDictionary> dict;
                    item->GetProperties(dict.m_Ptr);

                    std::string symbolType;
                    Utils::ReadString(dict, kLibProp_SymbolType_DictKey, symbolType);

                    FCM::U_Int32 valueLen;
                    FCM::FCMDictRecTypeID type;
                    FCM::Result res = dict->GetInfo("SourceFilePath", type, valueLen);

                    if (symbolType != "MovieClip") continue;

                    uint16_t symbolIdentifer = resources.GetIdentifer(itemName);

                    if (symbolIdentifer != UINT16_MAX) continue;

                    if (resources.context.window->ui->aboutToExit) {
                        resources.context.close();
                        return;
                    }

                    resources.AddSymbol(itemName, symbolItem, true);
                }
            }
        };
        
        uint16_t ResourcePublisher::AddLibraryItem(
            DOM::ILibraryItem* item,
            bool hasName
        ) {
            FCM::StringRep16 itemNamePtr;
            item->GetName(&itemNamePtr);
            u16string itemName = (const char16_t*)itemNamePtr;
            context.falloc->Free(itemNamePtr);

            FCM::AutoPtr<DOM::LibraryItem::ISymbolItem> symbolItem = item;
            FCM::AutoPtr<DOM::LibraryItem::IMediaItem> mediaItem = item;

            if (symbolItem) {
                return AddSymbol(itemName, symbolItem, hasName);
            }
            else if (mediaItem) {
                pSharedShapeWriter shape = m_writer->AddShape();
                cv::Mat image;
                shapeGenerator->GetImage(mediaItem, image);

                shape->AddGraphic(image, { 1, 0, 0, 1, 0, 0 });

                uint16_t identifer = m_id++;
                shape->Finalize(identifer);

                m_symbolsDict.push_back(
                    { itemName, identifer }
                );

                return identifer;
            }

            // In cases where item is not media and not symbol
            return FCM_EXPORT_FAILED;
        }

        uint16_t ResourcePublisher::AddSymbol(
            u16string name,
            DOM::LibraryItem::ISymbolItem* item,
            bool hasName
        ) {

#ifdef DEBUG
            context.trace("Symbol: %s", Utils::ToUtf8(name).c_str());
#endif // DEBUG

            FCM::AutoPtr<DOM::ITimeline> timeline;
            item->GetTimeLine(timeline.m_Ptr);

            if (hasName) {
                return AddMovieclip(name, timeline, hasName);
            }
            else {
                return AddShape(name, timeline);
            }

        };

        uint16_t ResourcePublisher::AddMovieclip(
            u16string name,
            FCM::AutoPtr<DOM::ITimeline> timeline,
            bool hasName
        ) {
            pSharedMovieclipWriter movieclip = m_writer->AddMovieclip();
            timelineBuilder->Generate(movieclip, timeline);

            uint16_t identifer = m_id++;
            m_symbolsDict.push_back(
                { name, identifer }
            );

            if (hasName) {
                movieclip->Finalize(identifer, m_fps, name);
            }
            else {
                movieclip->Finalize(identifer, m_fps, u"");
            }

            return identifer;
        };

        uint16_t ResourcePublisher::AddShape(
            u16string name,
            DOM::ITimeline* timeline
        ) {
            bool isShape = ShapeGenerator::Validate(timeline);
            
            if (!isShape) {
                return AddMovieclip(name, timeline, false);
            }

            pSharedShapeWriter shape = m_writer->AddShape();

            shapeGenerator->Generate(shape, timeline);

            uint16_t identifer = m_id++;

            shape->Finalize(identifer);
            m_symbolsDict.push_back(
                { name, identifer }
            );

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
            for (auto item : m_symbolsDict) {
                if (item.first == name) {
                    return item.second;
                }
            }

            return UINT16_MAX;
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

        void ResourcePublisher::AddCachedBitmap(u16string name, cv::Mat image) {
            m_imageSymbolsDataDict.push_back({ name, image });
        }

        bool ResourcePublisher::GetCachedBitmap(u16string name, cv::Mat& result) {
            for (pair<u16string, cv::Mat>& bitmap : m_imageSymbolsDataDict) {
                if (name == bitmap.first) {
                    result = bitmap.second;
                    return true;
                }
            }

            return false;
        }

        void ResourcePublisher::InitDocument(uint8_t fps) {
            m_fps = fps;

            m_symbolsDict.clear();
            m_modifierDict.clear();
            m_imageSymbolsDataDict.clear();
        }

        void ResourcePublisher::Finalize() {
            m_writer->Finalize();
        }
	}
}