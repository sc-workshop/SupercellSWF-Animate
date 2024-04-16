#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "Module/PluginContext.h"

// FCM stuff
#include "AnimateSDK/app/DOM/IFLADocument.h"
#include "AnimateSDK/core/common/FCMTypes.h"

// Timeline
#include "AnimateSDK/app/DOM/ITimeline1.h"
#include "AnimateSDK/app/DOM/ILayer2.h"

// Symbol
#include "AnimateSDK/app/DOM/ILibraryItem.h"
#include "AnimateSDK/app/DOM/LibraryItem/ISymbolItem.h"
#include "AnimateSDK/app/DOM/LibraryItem/IMediaItem.h"
#include "AnimateSDK/app/DOM/LibraryItem/IFolderItem.h"

// Image processing
#include <opencv2/opencv.hpp>

// Modifier
#include <SupercellFlash/objects/MovieClipModifier.h>

// Generators
#include "ResourcePublisher/SymbolGenerator/GraphicGenerator.h"
#include "ResourcePublisher/SymbolGenerator/MovieClipGenerator.h"
#include "Module/Symbol/SymbolContext.h"

#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledElement.h"
#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/TextElement.h"

// Writers
#include "ResourcePublisher/Writer/Shared/SharedWriter.h"
#include "ResourcePublisher/Writer/Writer.h"

// Symbol
#include "AnimateSDK/app/DOM/ILibraryItem.h"
#include "AnimateSDK/app/DOM/LibraryItem/ISymbolItem.h"

namespace sc {
	namespace Adobe {
		class ResourcePublisher {
		public:
			void Publish();

		private:
			static void GetItems(
				FCM::FCMListPtr libraryItems,
				std::vector<FCM::AutoPtr<DOM::ILibraryItem>>& paths
			);

		public:
			SharedWriter& m_writer;

			MovieClipGeneator movieClipGenerator;
			GraphicGenerator graphicGenerator;

			// Name  /  Id
			std::unordered_map<std::u16string, uint16_t> m_symbolsData;

			// Type / Id
			std::vector<std::pair<MaskedLayerState, uint16_t>> m_modifierDict;

			// Info / Id
			std::vector<std::pair<TextElement, uint16_t>> m_textfieldDict;

			// Shape / Id
			std::vector<std::pair<FilledElement, uint16_t>> m_filledShapeDict;

			uint32_t m_id = 0;
			uint8_t m_current_fps = 30;

		public:
			ResourcePublisher(SharedWriter& writer) :
				m_writer(writer), movieClipGenerator(*this), graphicGenerator(*this)
			{}

			uint16_t AddLibraryItem(
				SymbolContext& symbol,
				DOM::ILibraryItem* item
			);

			uint16_t AddSymbol(
				SymbolContext& symbol,
				DOM::LibraryItem::ISymbolItem* item
			);

			uint16_t AddMovieclip(
				SymbolContext& symbol,
				FCM::AutoPtr<DOM::ITimeline1> timeline
			);

			uint16_t AddShape(
				SymbolContext& symbol,
				FCM::AutoPtr <DOM::ITimeline1> timeline
			);

			uint16_t AddModifier(
				MaskedLayerState type
			);

			uint16_t AddTextField(
				SymbolContext& symbol,
				TextElement& field
			);

			uint16_t AddFilledElement(
				SymbolContext& symbol,
				const std::vector<FilledElement>& shape
			);

			uint16_t GetIdentifer(
				const std::u16string& name
			);

			uint16_t GetIdentifer(
				MaskedLayerState type
			);

			uint16_t GetIdentifer(
				TextElement field
			);

			uint16_t GetIdentifer(
				FilledElement shape
			);
		};
	}
}