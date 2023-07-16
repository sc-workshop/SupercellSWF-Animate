#pragma once

#include <vector>
#include <string>
#include <unordered_map>

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

#include "Publisher/TimelineBuilder/FrameElements/TextField.h"
#include "Publisher/TimelineBuilder/FrameElements/FilledShape.h"

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

        private:
            static void GetItemsPaths(AppContext& resources, FCM::FCMListPtr libraryItems, std::vector<std::u16string>& paths);

        public:
            SharedWriter* m_writer;

            shared_ptr<TimelineBuilder> timelineBuilder;
            shared_ptr<ShapeGenerator> shapeGenerator;

                               // Name  /  Id
            std::unordered_map<u16string, uint16_t> m_symbolsData;

                                // Name / Image
            std::unordered_map<u16string, cv::Mat> m_imagesData;

                                                // Type / Id
            std::vector<pair<sc::MovieClipModifier::Type, uint16_t>> m_modifierDict;

                                  // Info / Id
            std::vector<pair<TextFieldInfo, uint16_t>> m_textfieldDict;

                                // Shape / Id
            std::vector<pair<FilledShape, uint16_t>> m_filledShapeDict;

                                // Name / Usage count
            std::unordered_map<u16string, uint32_t> m_symbolsUsage;

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
                std::u16string name,
                FCM::AutoPtr<DOM::ILibraryItem> item
            );

            uint16_t AddSymbol(
                std::u16string name,
                DOM::LibraryItem::ISymbolItem* item,
                std::string symbolType
            );

            uint16_t AddMovieclip(
                u16string name,
                FCM::AutoPtr<DOM::ITimeline> timeline
            );

            uint16_t AddShape(
                u16string name,
                FCM::AutoPtr < DOM::ITimeline> timeline
            );

            uint16_t AddModifier(
                sc::MovieClipModifier::Type type
            );

            uint16_t AddTextField(
                TextFieldInfo field
            );

            uint16_t AddFilledShape(
                FilledShape shape
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

            uint16_t GetIdentifer(
                FilledShape shape
            );


            void AddCachedBitmap(u16string name, cv::Mat image);

            bool GetCachedBitmap(u16string name, cv::Mat& result);

            void InitDocument(uint8_t fps);

            uint32_t& GetSymbolUsage(std::u16string name);

            void Finalize(std::vector<std::u16string> exports);
		};
	}
}