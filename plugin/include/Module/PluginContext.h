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
					throw PluginException("TID_FCM_SERVICE_INIT_FAILED");
				}

				return (FCM::AutoPtr<T>)service;
			};

			std::string languageCode();

			template <class ... Args>
			void print(const char* message, Args ... args) {
				char buffer[1024];
				std::vsnprintf(buffer, 1024, fmt, ...args);

				std::string message(buffer);

				console->Trace((FCM::CStringRep16)Localization::ToUtf16(message + "\n").c_str());
			}

			template <class ... Args>
			void print(const uint16_t* message, Args ... args)
			{
				wchar_t buffer[1024];
				std::vswprintf(buffer, 1024, fmt, ...args);

				std::wstring message(buffer);
				message += L"\n";

				console->Trace((FCM::CStringRep16)message.c_str());
			}

			template <class ... Args>
			void print(const std::u16string& message, Args ... args)
			{
				print(message.c_str(), ...args)
			}

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