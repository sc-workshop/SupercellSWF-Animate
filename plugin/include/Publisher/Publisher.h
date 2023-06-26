#pragma once

#include "PluginConfiguration.h"
#include "Module/FCM_Identifiers.h"
#include "Module/Version.h"
#include "Publisher/IPublisher.h"

// FCM stuff
#include <FCMTypes.h>

// Publisher interface
#include <Publisher/IPublisher.h>

// Export config
#include "Module/Config.h"

#include "Publisher/ResourcePublisher.h"

#include <filesystem>
namespace fs = std::filesystem;

//using namespace Publisher;

namespace sc {
	namespace Adobe {
		class SCPublisher : public Publisher::IPublisher, public FCM::FCMObjectBase
		{
		public:
			virtual FCM::PIFCMCallback GetCallback() = 0;

			static FCM::U_Int32 GetVersion() { return (FCM::U_Int32)PLUGIN_VERSION; }

			static FCM::FCMInterfaceMap* GetInterfaceMap()
			{
				static FCM::FCMInterfaceMap _pInterfaceMap[] = {
					{
						FCM::IID_IFCMUnknown,
						((FCM::S_Int64)(static_cast<FCM::IFCMUnknown*>((SCPublisher*)1)) - 1),
						((FCM::_FCM_CREATORARGFUNC*)0)
					},
					{
						Publisher::IID_IPublisher,
						((FCM::S_Int64)(static_cast<Publisher::IPublisher*>((SCPublisher*)1)) - 1),
						((FCM::_FCM_CREATORARGFUNC*)0)
					},
					{
						FCM::FCMIID_NULL,
						0,
						((FCM::_FCM_CREATORARGFUNC*)0)
					 }
				};

				return _pInterfaceMap;
			}

		public:
			FCM::Result _FCMCALL Publish(DOM::PIFLADocument document, const FCM::PIFCMDictionary publishSettings, const FCM::PIFCMDictionary config); 

			FCM::Result _FCMCALL Publish(
				DOM::PIFLADocument document,
				DOM::PITimeline pTimeline,
				const Exporter::Service::RANGE& frameRange,
				const FCM::PIFCMDictionary publishSettings,
				const FCM::PIFCMDictionary config
			) { 
				return FCM_SERVICE_NOT_FOUND;
			}

			FCM::Result _FCMCALL ClearCache() { return FCM_SERVICE_NOT_FOUND; };
		};

		FCM::Result RegisterPublisher(FCM::PIFCMDictionary plugins, FCM::FCMCLSID docId);
	}
}