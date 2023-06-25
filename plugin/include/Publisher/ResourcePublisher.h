#pragma once

#include <vector>
#include <string>

#include "module/AppContext.h"

// FCM stuff
#include "DOM/IFLADocument.h"
#include "FCMTypes.h"

// Timeline
#include "DOM/ILayer2.h"

// Symbol
#include "DOM/ILibraryItem.h"
#include "DOM/LibraryItem/ISymbolItem.h"
#include "DOM/LibraryItem/IMediaItem.h"
#include "DOM/LibraryItem/IFolderItem.h"

// Image processing
#include <opencv2/opencv.hpp>

// Modifier
#include <SupercellFlash/objects/MovieClipModifier.h>

// Generators
#include "Publisher/ShapeGenerator.h"
#include "Publisher/TimelineBuilder/Builder.h"

#include "Publisher/TimelineBuilder/TextField.h"

// Writers
#include "Publisher/Shared/SharedWriter.h"
#include "Publisher/JSON/JSONWriter.h"
#include "Publisher/SWF/Writer.h"

// Symbol
#include "DOM/ILibraryItem.h"

#include "DOM/LibraryItem/ISymbolItem.h"

namespace sc {
	namespace Adobe {
		class ResourcePublisher {
        public:
            static void Publish(AppContext& context);
            static void PublishItems(FCM::FCMListPtr libraryItems, ResourcePublisher& resources);

        public:
            SharedWriter* m_writer;

            shared_ptr<TimelineBuilder> timelineBuilder;
            shared_ptr<ShapeGenerator> shapeGenerator;

                         // Name  /  Id
            vector<pair<u16string, uint16_t>> m_symbolsDict;

                         // Name / Image
            vector<pair<u16string, cv::Mat>> m_imageSymbolsDataDict;

                                            // Type / Id
            vector<pair<sc::MovieClipModifier::Type, uint16_t>> m_modifierDict;

                             // Info / Id
            vector<pair<TextFieldInfo, uint16_t>> m_textfieldDict;

            uint32_t m_id = 0;
            uint8_t m_fps = 24;

        public:
            AppContext& context;

            ResourcePublisher(AppContext& app, SharedWriter* writer):
                context(app),
                m_writer(writer)
            {
                timelineBuilder = shared_ptr<TimelineBuilder>(new TimelineBuilder(*this));
                shapeGenerator = shared_ptr<ShapeGenerator>(new ShapeGenerator(*this));
            }


            uint16_t AddLibraryItem(
                DOM::ILibraryItem* item,
                bool hasName = false
            );

            uint16_t AddSymbol(
                u16string name,
                DOM::LibraryItem::ISymbolItem* item,
                bool hasName = false
            );

            uint16_t AddMovieclip(
                u16string name,
                FCM::AutoPtr<DOM::ITimeline> timeline,
                bool hasName
            );

            uint16_t AddShape(
                u16string name,
                DOM::ITimeline* timeline
            );

            uint16_t AddModifier(
                sc::MovieClipModifier::Type type
            );

            uint16_t AddTextField(
                TextFieldInfo field
            );

            uint16_t GetIdentifer(
                u16string name
            );

            uint16_t GetIdentifer(
                sc::MovieClipModifier::Type type
            );

            uint16_t GetIdentifer(
                TextFieldInfo field
            );

            void AddCachedBitmap(u16string name, cv::Mat image);

            bool GetCachedBitmap(u16string name, cv::Mat& result);

            void InitDocument(uint8_t fps);

            void Finalize();
		};
	}
}