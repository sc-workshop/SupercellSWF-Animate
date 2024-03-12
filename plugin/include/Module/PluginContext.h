#pragma once

#include "AnimateSDK/core/common/FCMTypes.h"
#include "AnimateSDK/core/common/FCMPluginInterface.h"

#include "AnimateSDK/app/ApplicationFCMPublicIDs.h"
#include "AnimateSDK/app/Application/Service/IApplicationService.h"
#include "AnimateSDK/app/Application/Service/IOutputConsoleService.h"

#include <string>
#include <stdarg.h>
#include <exception>
#include <thread>

#include "exception/GeneralRuntimeException.h"

#include "Module/Localization.h"

#include "Window/PluginWindowApp.h"

namespace sc {
	namespace Adobe {
		class PluginContext {
		public:
			static PluginContext& Instance()
			{
				static PluginContext singleton;
				return singleton;
			}

		private:
			PluginContext() {}
			~PluginContext() {}
			PluginContext(const PluginContext&);
			PluginContext& operator=(const PluginContext&);

			// Progress Window
			PluginWindowApp* m_app = nullptr;

		public:
			void UpdateCallback(FCM::PIFCMCallback callback);

		public:
			// FCM Plugin Context
			FCM::PIFCMCallback callback = nullptr;

			// FCM Memory Control
			FCM::AutoPtr<FCM::IFCMCalloc> falloc = nullptr;

			// Raw Program Console Implementation
			FCM::AutoPtr<Application::Service::IOutputConsoleService> console = nullptr;

			// App Context
			FCM::AutoPtr<Application::Service::IApplicationService> appService = nullptr;

			// Plugin Localization
			Localization locale;

		public:
			template<typename T>
			FCM::AutoPtr<T> getService(FCM::SRVCID id) {
				FCM::AutoPtr<FCM::IFCMUnknown> service;

				FCM::Result res = callback->GetService(id, service.m_Ptr);
				if (FCM_FAILURE_CODE(res)) {
					throw GeneralRuntimeException("Failed to initialize service");
				}

				return (FCM::AutoPtr<T>)service;
			};

			std::string languageCode();

			void print(const char* message, ...);
			void print(const wchar_t* message, ...);

			bool& initializeWindow();
			void destroyWindow();
			PluginWindow* window();

		public:
			enum class PathType
			{
				Module, // Path to Binaries
				Extension, // Path to Extenson Root Folder
				Locale,	// Path to folder with locales
				Assets  // Path to folder with assets files
			};

			static fs::path CurrentPath(PathType type = PathType::Extension);
		};
	}
}