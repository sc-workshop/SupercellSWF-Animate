#include "Publisher/ResourcePublisher.h"

namespace sc {
	namespace Adobe {
        
        Result ResourcePublisher::AddLibraryItem(
            DOM::ILibraryItem* item,
            uint16_t& identifer,
            bool addExportName
        ) {
            Result res = FCM_SUCCESS;

            StringRep16 itemName;
            res = item->GetName(&itemName);
            FCM_SUCCESS;

            std::string itemNameStr = Utils::ToString(itemName, m_callback);

            AutoPtr<DOM::LibraryItem::ISymbolItem> symbolItem = item;
            AutoPtr<DOM::LibraryItem::IMediaItem> mediaItem = item;

            if (symbolItem) {
                return AddSymbol(itemNameStr, symbolItem, identifer, addExportName);
            }
            else if (mediaItem) {
                pSharedShapeWriter shape = m_writer->AddShape();
                cv::Mat image;
                res = shapeGenerator.GetImage(mediaItem, image);
                FCM_CHECK;

                res = shape->AddGraphic(image, { 1, 0, 0, 1, 0, 0 });
                FCM_CHECK;

                identifer = m_id;
                m_id++;
                shape->Finalize(identifer);

                m_symbolsDict.push_back(
                    std::pair(itemNameStr, identifer)
                );

                return res;
            }

            // In cases where item is not media and not symbol
            return FCM_EXPORT_FAILED;
        }

        Result ResourcePublisher::AddSymbol(
            std::string name,
            DOM::LibraryItem::ISymbolItem* item,
            uint16_t& identifer,
            bool addExportName
        ) {
            Result res;

            debugLog("Symbol: %s", name.c_str());

            AutoPtr<DOM::ITimeline> timeline;
            res = item->GetTimeLine(timeline.m_Ptr);
            FCM_CHECK;

            bool isShape = false;
            ShapeGenerator::Validate(timeline, isShape);

            if (addExportName) {
                return AddMovieclip(name, timeline, identifer);
            }
            else {
                return AddShape(name, timeline, identifer);
            }

        };

        Result ResourcePublisher::AddMovieclip(
            std::string name,
            DOM::ITimeline* timeline,
            uint16_t& identifer
        ) {
            Result res = FCM_SUCCESS;

            pSharedMovieclipWriter movieclip = m_writer->AddMovieclip();

            res = frameGenerator.Generate(
                movieclip,
                timeline
            );
            FCM_CHECK;

            identifer = m_id;
            m_id++;

            movieclip->Finalize(identifer, 24, name);

            m_symbolsDict.push_back(
                std::pair(name, identifer)
            );

            return res;
        };

        Result ResourcePublisher::AddShape(
            std::string name,
            DOM::ITimeline* timeline,
            uint16_t& identifer
        ) {
            Result res = FCM_SUCCESS;

            bool isShape = true;
            res = ShapeGenerator::Validate(timeline, isShape);
            FCM_CHECK;

            if (!isShape) {
                return AddMovieclip(name, timeline, identifer);
            }

            pSharedShapeWriter shape = m_writer->AddShape();

            res = shapeGenerator.Generate(shape, timeline);
            FCM_CHECK;

            identifer = m_id;
            m_id++;

            shape->Finalize(identifer);

            m_symbolsDict.push_back(
                std::pair(name, identifer)
            );

            return res;
        }

        Result ResourcePublisher::GetIdentifer(std::string name, uint16_t& identifer) {
            for (std::pair<std::string, U_Int16> item : m_symbolsDict) {
                if (item.first == name) {
                    identifer = item.second;
                    return FCM_SUCCESS;
                }
            }

            identifer = UINT16_MAX;
            return FCM_SUCCESS;
        }

        Result ResourcePublisher::Finalize() {

            m_writer->Finalize();

            return FCM_SUCCESS;
        }

        Result ResourcePublisher::Init(SharedWriter* writer, PIFCMCallback callback) {
            m_symbolsDict.clear();
            m_id = 0;
            m_callback = callback;
            m_writer = writer;

            console.Init("ResourcePublisher", m_callback);
            frameGenerator.Init(m_callback, this);
            shapeGenerator.Init(m_callback, this);

            return FCM_SUCCESS;
        }
	}
}