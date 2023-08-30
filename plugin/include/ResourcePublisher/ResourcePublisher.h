#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "module/Context.h"

// FCM stuff
#include "DOM/IFLADocument.h"
#include "FCMTypes.h"

// Timeline
#include "DOM/ITimeline1.h"
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
#include "ResourcePublisher/SymbolGenerator/GraphicGenerator.h"
#include "ResourcePublisher/SymbolGenerator/MovieClipGenerator.h"
#include "ResourcePublisher/SymbolGenerator/SymbolBehavior.h"

#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledShape.h"
#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/TextField.h"

// Writers
#include "ResourcePublisher/Writer/Shared/SharedWriter.h"
#include "ResourcePublisher/Writer/JSON/JSONWriter.h"
#include "ResourcePublisher/Writer/SWF/Writer.h"

// Symbol
#include "DOM/ILibraryItem.h"
#include "DOM/LibraryItem/ISymbolItem.h"

namespace sc {
	namespace Adobe {
		class ResourcePublisher {
		public:
			static void Publish(Context& context);

		private:
			static void GetItemsPaths(Context& context, FCM::FCMListPtr libraryItems, std::vector<std::u16string>& paths);

		public:
			SharedWriter* m_writer;

			shared_ptr<MovieClipGeneator> movieClipGenerator;
			shared_ptr<GraphicGenerator> graphicGenerator;

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
			Context& context;

			ResourcePublisher(Context& app, SharedWriter* writer) :
				context(app),
				m_writer(writer)
			{
				movieClipGenerator = shared_ptr<MovieClipGeneator>(new MovieClipGeneator(*this));
				graphicGenerator = shared_ptr<GraphicGenerator>(new GraphicGenerator(*this));
			}

			uint16_t AddLibraryItem(
				std::u16string name,
				FCM::AutoPtr<DOM::ILibraryItem> item
			);

			uint16_t AddSymbol(
				std::u16string name,
				DOM::LibraryItem::ISymbolItem* item,
				SymbolBehaviorInfo& symbolBehavior
			);

			uint16_t AddMovieclip(
				u16string name,
				FCM::AutoPtr<DOM::ITimeline1> timeline,
				SymbolBehaviorInfo& symbolBehavior
			);

			uint16_t AddShape(
				u16string name,
				FCM::AutoPtr <DOM::ITimeline1> timeline,
				SymbolBehaviorInfo& symbolBehavior
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