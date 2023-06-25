#pragma once

#include "DOM/IFLADocument.h"
#include "FCMTypes.h"

#include <string>
#include <stdarg.h>
#include <exception>
#include <thread>

#include "Localization.h"
#include "Module/Config.h"

#include "FCMPluginInterface.h"
#include "Application/Service/IApplicationService.h"
#include "Application/Service/IOutputConsoleService.h"
#include "ApplicationFCMPublicIDs.h"

#include "Ui/PluginUI.h"

namespace sc {
	namespace Adobe {
		class AppContext {
			FCM::PIFCMCallback m_callback;

		public:
			AppContext(FCM::PIFCMCallback callback, DOM::IFLADocument* document, const FCM::PIFCMDictionary settings);

			~AppContext();

		public:
			// Public properties

			// Current document
			DOM::IFLADocument* document;

			// FCM memory control
			FCM::AutoPtr<FCM::IFCMCalloc> falloc;

			// Module localization
			Localization locale;

			// Publish settings
			Config config;

			// Import / export window
			PluginUI* window = nullptr;

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

			void close();
		};
	}
}