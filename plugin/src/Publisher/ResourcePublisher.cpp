#include "Publisher/ResourcePublisher.h"

namespace sc {
	namespace Adobe {
        
        uint16_t ResourcePublisher::AddLibraryItem(
            DOM::ILibraryItem* item,
            bool hasName
        ) {
            StringRep16 itemNamePtr;
            item->GetName(&itemNamePtr);
            u16string itemName = (const char16_t*)itemNamePtr;
            context.falloc->Free(itemNamePtr);

            AutoPtr<DOM::LibraryItem::ISymbolItem> symbolItem = item;
            AutoPtr<DOM::LibraryItem::IMediaItem> mediaItem = item;

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

            AutoPtr<DOM::ITimeline> timeline;
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
            AutoPtr<DOM::ITimeline> timeline,
            bool hasName
        ) {
            pSharedMovieclipWriter movieclip = m_writer->AddMovieclip();
            timelineBuilder->Generate(movieclip, timeline);

            uint16_t identifer = m_id++;
            m_symbolsDict.push_back(
                { name, identifer }
            );

            if (hasName) {
                movieclip->Finalize(identifer, 24, name);
            }
            else {
                movieclip->Finalize(identifer, 24, u"");
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
            for (uint16_t i = 0; m_modifierDict.size() > i; i++) {
                if (m_modifierDict[i].first == type) {
                    return m_modifierDict[i].second;
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

        void ResourcePublisher::Finalize() {
            m_id = 0;
            m_symbolsDict.clear();
            m_modifierDict.clear();

            m_writer->Finalize();
        }
	}
}