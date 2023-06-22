#pragma once

#include "FCMTypes.h"

#include <string>
#include <stdarg.h>
#include <exception>

#include "Localization.h"
#include "Publisher/PublisherConfig.h"

#include "FCMPluginInterface.h"
#include "Application/Service/IApplicationService.h"
#include "Application/Service/IOutputConsoleService.h"
#include "ApplicationFCMPublicIDs.h"

#include "Module/Window.h"

namespace sc {
	namespace Adobe {
		class AppContext {
			FCM::PIFCMCallback m_callback;

		public:
			AppContext(FCM::PIFCMCallback callback, const FCM::PIFCMDictionary settings);

			~AppContext();

		public:
			// Public properties

			// FCM memory control
			FCM::AutoPtr<FCM::IFCMCalloc> falloc;

			// Module localization
			LocaleInterface locale;

			// Publish settings
			PublisherConfig config;

			// Import / export window
			Window* window = nullptr;

		public:
			// Functions

			template<typename T>
			FCM::AutoPtr<T> getService(FCM::SRVCID id) {
				FCM::AutoPtr<FCM::IFCMUnknown> unknownService;

				FCM::Result res = m_callback->GetService(id, unknownService.m_Ptr);
				if (FCM_FAILURE_CODE(res)) {
					throw exception("Failed to initialize service");
				}

				FCM::AutoPtr<T> service = unknownService;
				return service;
			};

			std::string languageCode();

			void trace(const char* fmt, ...);

			// Publish modes

			//void startExport(DOM::PIFLADocument document);

			//void startImport();
		};
	}
}