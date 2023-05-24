#pragma once

#include <FCMTypes.h>
#include <FCMPluginInterface.h>

#include <FillStyle/ISolidFillStyle.h>
#include <FillStyle/IBitmapFillStyle.h>
#include <FrameElement/ITextStyle.h>
#include <Service/Shape/IPath.h>
#include <Utils/DOMTypes.h>

#include "DOM/ILibraryItem.h"
#include "DOM/ILayer2.h"
#include <DOM/LibraryItem/ISymbolItem.h>
#include <DOM/LibraryItem/IMediaItem.h>
#include <DOM/MediaInfo/IBitmapInfo.h>

#include "io/Console.h"
#include "Shared/SharedWriter.h"
#include "FrameGenerator.h"

#include <vector>
#include <string>

using namespace FCM;

namespace DOM
{
    namespace FrameElement
    {
        FORWARD_DECLARE_INTERFACE(IShape);
        FORWARD_DECLARE_INTERFACE(IClassicText);
    }
}

namespace sc {
	namespace Adobe {
		class ResourcePublisher {
            PIFCMCallback m_callback = nullptr;
            SharedWriter* m_writer = nullptr;
            FrameGenerator frameGenerator;
                                    // Name  /  Id
            std::vector<std::pair<std::string, U_Int16>> m_symbolsDict;

            U_Int16 m_id = 0;

            Console console;

        public:
            ResourcePublisher() { }
            ~ResourcePublisher() { }

            U_Int16 _FCMCALL AddLibraryItem(
                DOM::ILibraryItem* item
            ) {
                StringRep16 itemName;
                item->GetName(&itemName);
                std::string itemNameStr = Utils::ToString(itemName, m_callback);

                AutoPtr<DOM::LibraryItem::ISymbolItem> symbolItem = item;
                AutoPtr<DOM::LibraryItem::IMediaItem> mediaItem = item;

                if (symbolItem) {
                    return AddSymbol(itemNameStr, symbolItem);
                }
                else if (mediaItem) {
                    return AddSingleBitmapObject(itemNameStr, mediaItem);
                }

                return UINT16_MAX;
            }

            U_Int16 _FCMCALL AddSymbol(
                std::string name, DOM::LibraryItem::ISymbolItem* item) {
                Result res;

                debugLog("Symbol: %s", name.c_str());

                AutoPtr<DOM::ITimeline> timeline;
                res = item->GetTimeLine(timeline.m_Ptr);
                FCM_CHECK;

                U_Int32 symbolDuration;
                timeline->GetMaxFrameCount(symbolDuration);

                if (symbolDuration == 1) {
                    return AddShape(name, timeline);
                }

                return AddMovieclip(name, timeline);
            };

            U_Int16 AddMovieclip(std::string name, DOM::ITimeline* timeline) {
                SharedMovieclipWriter* movieclip = m_writer->AddMovieclip();

                frameGenerator.Generate(
                    movieclip,
                    timeline
                );

                U_Int16 symbolIdentifer = m_id;
                m_id++;

                movieclip->Finalize(m_id, 24, name);

                m_symbolsDict.push_back(
                    std::pair(name, symbolIdentifer)
                );

                return FCM_SUCCESS;
            }

            U_Int16 _FCMCALL AddShape(
                std::string name,
                DOM::ITimeline* timeline
            ) {
                Result res;

                FCM::FCMListPtr layers;
                res = timeline->GetLayers(layers.m_Ptr);
                FCM_CHECK;

                FCM::U_Int32 layerCount = 0;
                res = layers->Count(layerCount);
                FCM_CHECK;

                bool isValidShape = false;

                // Make sure it's a shape
                for (uint32_t layerIndex = 0; layerCount > layerIndex; layerIndex++) {
                    AutoPtr<DOM::ILayer2> layer = layers[layerIndex];

                    AutoPtr<IFCMUnknown> unknownLayer;
                    layer->GetLayerType(unknownLayer.m_Ptr);
                    AutoPtr<DOM::Layer::ILayerNormal> normalLayer = unknownLayer;
                    
                    if (!normalLayer) goto VALID_END;

                    FCM::FCMListPtr keyframes;
                    normalLayer->GetKeyFrames(keyframes.m_Ptr);

                    AutoPtr<DOM::IFrame> keyframe = keyframes[0];

                    FCM::FCMListPtr shapeElements;
                    keyframe->GetFrameElements(shapeElements.m_Ptr);

                    U_Int32 elementsCount = 0;
                    shapeElements->Count(elementsCount);

                    for (U_Int32 elementIndex = 0; elementsCount > elementIndex; elementIndex++) {
                        AutoPtr<DOM::FrameElement::IInstance> frameElement = shapeElements[elementIndex];

                        if (!frameElement) goto VALID_END;

                        AutoPtr<DOM::ILibraryItem> item;
                        frameElement->GetLibraryItem(item.m_Ptr);

                        if (!item) goto VALID_END;

                        AutoPtr<DOM::LibraryItem::IMediaItem> media = item;

                        if (!media) goto VALID_END;
                    }

                    isValidShape = true;
                }

            VALID_END:
                if (!isValidShape) {
                    return AddMovieclip(name, timeline);
                }

                SharedShapeWriter* shape = m_writer->AddShape();

                for (uint32_t layerIndex = 0; layerCount > layerIndex; layerIndex++) {
                    AutoPtr<DOM::ILayer2> layer = layers[layerIndex];

                    AutoPtr<IFCMUnknown> unknownLayer;
                    layer->GetLayerType(unknownLayer.m_Ptr);
                    AutoPtr<DOM::Layer::ILayerNormal> normalLayer = unknownLayer;

                    FCM::FCMListPtr keyframes;
                    normalLayer->GetKeyFrames(keyframes.m_Ptr);

                    AutoPtr<DOM::IFrame> keyframe = keyframes[0];

                    FCM::FCMListPtr shapeElements;
                    keyframe->GetFrameElements(shapeElements.m_Ptr);

                    U_Int32 elementsCount = 0;
                    shapeElements->Count(elementsCount);

                    for (U_Int32 elementIndex = 0; elementsCount > elementIndex; elementIndex++) {
                        // Bitmap transform
                        AutoPtr<DOM::FrameElement::IFrameDisplayElement> frameElement = shapeElements[elementIndex];

                        DOM::Utils::MATRIX2D transformMatrix;
                        frameElement->GetMatrix(transformMatrix);

                        // Bitmap itself
                        AutoPtr<DOM::FrameElement::IInstance> instance = shapeElements[elementIndex];

                        AutoPtr<DOM::ILibraryItem> item;
                        instance->GetLibraryItem(item.m_Ptr);

                        AutoPtr<DOM::LibraryItem::IMediaItem> media = item;

                        AutoPtr<IFCMUnknown> unknownMedia;
                        media->GetMediaInfo(unknownMedia.m_Ptr);

                        AutoPtr<DOM::MediaInfo::IBitmapInfo> bitmapInfo = unknownMedia;

                        shape->AddGraphic(media, transformMatrix);
                    }
                }

                U_Int16 symbolIdentifer = m_id;
                m_id++;

                shape->Finalize(symbolIdentifer);

                m_symbolsDict.push_back(
                    std::pair(name, symbolIdentifer)
                );

                return symbolIdentifer;
            }

            U_Int16 AddSingleBitmapObject(std::string name, DOM::LibraryItem::IMediaItem* item) {
                SharedShapeWriter* shape = m_writer->AddShape();
                shape->AddGraphic(item, DOM::Utils::MATRIX2D());

                U_Int16 symbolIdentifer = m_id;
                m_id++;
                shape->Finalize(symbolIdentifer);

                m_symbolsDict.push_back(
                    std::pair(name, symbolIdentifer)
                );

                return symbolIdentifer;
            }

            Result GetIdentifer(std::string name, U_Int16& result) {
                for (std::pair<std::string, U_Int16> item : m_symbolsDict) {
                    if (item.first == name) {
                        result = item.second;
                        return FCM_SUCCESS;
                    }
                }

                result = UINT16_MAX;
                return FCM_SUCCESS;
            }

            Result Finalize() {

                m_writer->Finalize();

                return FCM_SUCCESS;
            }

            Result Init(SharedWriter* writer, PIFCMCallback callback) {
                m_symbolsDict.clear();
                m_id = 0;
                m_callback = callback;
                m_writer = writer;

                console.Init("ResourcePublisher", m_callback);
                frameGenerator.Init(m_callback, this);

                return FCM_SUCCESS;
            }
		};
	}
}