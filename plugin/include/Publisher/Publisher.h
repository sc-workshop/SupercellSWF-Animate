#pragma once

#include <FCMTypes.h>
#include <Utils/DOMTypes.h>
#include <PluginConfiguration.h>
#include <Publisher/IPublisher.h>

#include "DOM/ILibraryItem.h"
#include "DOM/LibraryItem/IFolderItem.h"
#include "DOM/LibraryItem/ISymbolItem.h"

#include <string>

#include "Version.h"
#include "io/Console.h"
#include "Ids.h"
#include "Utils.h"
#include "ResourcePublisher.h"
#include "Macros.h"
#include "PublisherConfig.h"

#include "Publisher/JSON/JSONWriter.h"
#include "Publisher/SWF/Writer.h"

#include <filesystem>
namespace fs = std::filesystem;

using namespace FCM;
using namespace Publisher;
using namespace Exporter::Service;
using namespace DOM::Service::Tween;

namespace sc {
	namespace Adobe {
		class Publisher : public IPublisher, public FCMObjectBase
		{
			BEGIN_INTERFACE_MAP(Publisher, PLUGIN_VERSION)
				INTERFACE_ENTRY(IPublisher)
			END_INTERFACE_MAP

			PublisherConfig m_config;
			ResourcePublisher m_resources;

			Console console;

		public:
			Result _FCMCALL Publish(DOM::PIFLADocument document, const PIFCMDictionary publishSettings, const PIFCMDictionary config); 

			Result _FCMCALL Publish(DOM::PIFLADocument document, DOM::PITimeline pTimeline, const Exporter::Service::RANGE& frameRange, const PIFCMDictionary publishSettings, const PIFCMDictionary config);

			FCM::Result _FCMCALL ClearCache() { return FCM_SUCCESS; }\

		private:
			FCM::Result ExportLibraryItems(FCM::FCMListPtr libraryItems);
		};

		FCM::Result RegisterPublisher(PIFCMDictionary plugins, FCM::FCMCLSID docId);
	}
}