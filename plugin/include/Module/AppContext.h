#pragma once

#include "FCMTypes.h"

#include <string>
#include <stdarg.h>

#include "Localization.h"
#include "Publisher/PublisherConfig.h"

#include "Application/Service/IApplicationService.h"
#include "Application/Service/IOutputConsoleService.h"
#include <ApplicationFCMPublicIDs.h>

using namespace std;
using namespace FCM;
using namespace Application::Service;

namespace sc {
	namespace Adobe {
		class AppContext {
			PIFCMCallback m_callback;

		public:
			AppContext(PIFCMCallback callback, const PIFCMDictionary settings) {
				if (!callback) {
					throw exception("Failed to initialize app interface");
				}
				else {
					m_callback = callback;
				}
				falloc = getService<IFCMCalloc>(SRVCID_Core_Memory);

				locale.Load(languageCode());
				if (settings) {
					config = PublisherConfig::FromDict(settings);
				}
			};

			~AppContext() {}

		public:
			// Public properties

			// FCM memory control
			AutoPtr<IFCMCalloc> falloc;

			// Module localization
			LocaleInterface locale;

			// Publish settings
			PublisherConfig config;

		public:
			// Functions

			template<typename T>
			AutoPtr<T> getService(SRVCID id) {
				AutoPtr<IFCMUnknown> unknownService;

				Result res = m_callback->GetService(id, unknownService.m_Ptr);
				if (FCM_FAILURE_CODE(res)) {
					throw exception("Failed to initialize service");
				}

				AutoPtr<T> service = unknownService;
				return service;
			};

			string languageCode();

			void trace(const char* fmt, ...);
		};
	}
}