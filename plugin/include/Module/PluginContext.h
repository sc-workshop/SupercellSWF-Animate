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
#include <fstream>
#include <sstream>

#include "Module/PluginException.h"
#include "Module/Localization.h"

#include "Window/PluginWindowApp.h"

#include <fmt/xchar.h>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/ostream_sink.h"

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
			PluginContext();
			~PluginContext() = default;
			PluginContext(const PluginContext&) = delete;
			PluginContext& operator=(const PluginContext&) = delete;

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

			// Plugin Logger
			std::shared_ptr<spdlog::logger> logger;

			// Log File for logger
			std::ofstream logger_file;

		public:
			template<typename T>
			FCM::AutoPtr<T> GetService(FCM::SRVCID id) {
				FCM::AutoPtr<FCM::IFCMUnknown> service;

				FCM::Result res = callback->GetService(id, service.m_Ptr);
				if (FCM_FAILURE_CODE(res)) {
					throw PluginException("TID_FCM_SERVICE_INIT_FAILED");
				}

				return (FCM::AutoPtr<T>)service;
			};

			template <class ... Args>
			void Trace(const char* message, Args ... args) {
				char buffer[1024];
				std::snprintf(buffer, 1024, message, args...);

				std::string result(buffer);
				console->Trace((FCM::CStringRep16)Localization::ToUtf16(result + "\n").c_str());
			}

			template <class ... Args>
			void Trace(const char16_t* message, Args ... args)
			{
				wchar_t buffer[1024];
				std::swprintf(buffer, 1024, (const wchar_t*)message, args...);

				std::wstring result(buffer);
				result += L"\n";

				console->Trace((FCM::CStringRep16)result.c_str());
			}

			template <class ... Args>
			void Trace(const std::u16string& message, Args ... args)
			{
				Trace(message.c_str(), args...);
			}

		public:
			std::string LanguageCode();

			void InitializeWindow();
			void DestroyWindow();
			PluginWindow* Window();

		public:
			enum class PathType
			{
				Module, // Path to Binaries
				Extension, // Path to Extenson Root Folder
				Locale,	// Path to folder with locales
				Assets  // Path to folder with assets files
			};

			static fs::path CurrentPath(PathType type = PathType::Extension);

			static std::string SystemInfo();
		};
	}
}