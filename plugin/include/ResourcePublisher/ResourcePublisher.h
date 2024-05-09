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

		private:
			// Shape / Id
			using FilledDictValue = std::pair<std::vector<FilledElement>, uint16_t>;
			using FilledDict = std::vector<FilledDictValue>;

			// Name  /  Id
			using SymbolDict = std::unordered_map<std::u16string, uint16_t>;

			// Type / Id
			using ModifierDict = std::unordered_map<MaskedLayerState, uint16_t>;

			// Info / Id
			using TextsDictValue = std::pair<TextElement, uint16_t>;
			using TextsDict = std::vector<TextsDictValue>;

		public:
			SharedWriter& m_writer;

			MovieClipGeneator movieClipGenerator;
			GraphicGenerator graphicGenerator;
			SymbolDict m_symbolsData;
			ModifierDict m_modifierDict;
			TextsDict m_textfieldDict;
			FilledDict m_filledShapeDict;

			uint32_t m_id = 0;
			uint8_t m_current_fps = 30;

		public:
			ResourcePublisher(SharedWriter& writer) :
				m_writer(writer), movieClipGenerator(*this), graphicGenerator(*this)
			{}

			uint16_t AddLibraryItem(
				SymbolContext& symbol,
				FCM::AutoPtr<DOM::ILibraryItem> item,
				bool required = false
			);

			uint16_t AddSymbol(
				SymbolContext& symbol,
				FCM::AutoPtr<DOM::LibraryItem::ISymbolItem> item,
				bool required = false
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
				const std::vector<FilledElement>& shape,
				bool required = false
			);

			uint16_t GetIdentifer(
				const std::u16string& name
			) const;

			uint16_t GetIdentifer(
				const MaskedLayerState& type
			) const;

			uint16_t GetIdentifer(
				const TextElement& field
			) const;

			uint16_t GetIdentifer(
				const std::vector<FilledElement>& shape
			) const;

		private:
			uint16_t AddMovieclip(
				SymbolContext& symbol,
				FCM::AutoPtr<DOM::ITimeline1> timeline,
				bool required = false
			);

			uint16_t AddShape(
				SymbolContext& symbol,
				FCM::AutoPtr <DOM::ITimeline1> timeline,
				bool required = false
			);
		};
	}
}