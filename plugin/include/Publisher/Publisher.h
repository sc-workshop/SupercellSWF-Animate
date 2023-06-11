#pragma once

#include "PluginConfiguration.h"
#include "Module/FCM_Identifiers.h"
#include "Module/Version.h"

// FCM stuff
#include <FCMTypes.h>

// Publisher interface
#include <Publisher/IPublisher.h>

// Export config
#include "Publisher/PublisherConfig.h"

// Writers
#include "Publisher/JSON/JSONWriter.h"
#include "Publisher/SWF/Writer.h"

// Symbol
#include "DOM/ILibraryItem.h"
#include "DOM/LibraryItem/IFolderItem.h"
#include "DOM/LibraryItem/ISymbolItem.h"

#include "Publisher/ResourcePublisher.h"

#include <filesystem>
namespace fs = std::filesystem;

using namespace FCM;
using namespace Publisher;

namespace sc {
	namespace Adobe {
		class Publisher : public IPublisher, public FCMObjectBase
		{
			BEGIN_INTERFACE_MAP(Publisher, PLUGIN_VERSION)
				INTERFACE_ENTRY(IPublisher)
			END_INTERFACE_MAP
		public:
			Result _FCMCALL Publish(DOM::PIFLADocument document, const PIFCMDictionary publishSettings, const PIFCMDictionary config); 

			Result _FCMCALL Publish(
				DOM::PIFLADocument document,
				DOM::PITimeline pTimeline,
				const Exporter::Service::RANGE& frameRange,
				const PIFCMDictionary publishSettings,
				const PIFCMDictionary config
			) { 
				return FCM_SERVICE_NOT_FOUND;
			}

			Result _FCMCALL ClearCache() { return FCM_SERVICE_NOT_FOUND; };

		private:
			void ExportLibraryItems(FCM::FCMListPtr libraryItems, ResourcePublisher& resources);
		};

		FCM::Result RegisterPublisher(PIFCMDictionary plugins, FCM::FCMCLSID docId);
	}
}