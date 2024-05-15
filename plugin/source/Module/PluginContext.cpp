#include "Module/PluginContext.h"

#include "PluginConfiguration.h"

#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#endif

namespace sc
{
	namespace Adobe
	{
		PluginContext::PluginContext()
		{
			const std::string log_name = std::string(DOCTYPE_UNIVERSAL_NAME "_export_log.txt");
			const fs::path log_path = fs::temp_directory_path() / log_name;

			if (fs::exists(log_path))
			{
				fs::remove(log_path);
			}

			logger_file = std::ofstream(log_path);
			auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(logger_file, true);

			logger = std::make_shared<spdlog::logger>(DOCTYPE_NAME, ostream_sink);
			logger->set_pattern("[%H:%M:%S] [%l] >> %v");
			spdlog::set_default_logger(logger);
		}

		void PluginContext::UpdateCallback(FCM::PIFCMCallback active_callback)
		{
			callback = active_callback;
			falloc = GetService<FCM::IFCMCalloc>(FCM::SRVCID_Core_Memory);
			console = GetService<Application::Service::IOutputConsoleService>(Application::Service::APP_OUTPUT_CONSOLE_SERVICE);
			appService = GetService<Application::Service::IApplicationService>(Application::Service::APP_SERVICE);
			locale.Load(LanguageCode());
		}

		std::string PluginContext::LanguageCode() {
			FCM::StringRep8 languageCodePtr;
			appService->GetLanguageCode(&languageCodePtr);

			std::string LanguageCode((const char*)languageCodePtr);

			falloc->Free(languageCodePtr);

			return LanguageCode;
		}

		void PluginContext::InitializeWindow()
		{
			m_app = new PluginWindowApp();
			wxApp::SetInstance(m_app);
		}

		void PluginContext::DestroyWindow()
		{
			if (m_app)
			{
				m_app = nullptr;
			}
		}

		PluginWindow* PluginContext::Window()
		{
			if (m_app)
			{
				return m_app->window;
			}

			return nullptr;
		}

		fs::path PluginContext::CurrentPath(PluginContext::PathType type)
		{
			fs::path modulePath;

#ifdef _WINDOWS
			char16_t* pathPtr = new char16_t[MAX_PATH];
			GetModuleFileName((HINSTANCE)&__ImageBase, (LPWSTR)pathPtr, MAX_PATH - 1);

			modulePath = fs::path(std::u16string((const char16_t*)pathPtr)).parent_path();
			delete[] pathPtr;
#else
#error Not implemented
#endif
			fs::path extensionPath = fs::path(modulePath / "../");
			fs::path assetsPath = fs::path(extensionPath / "resources");

			switch (type)
			{
			case PluginContext::PathType::Module:
				return modulePath;
			case PluginContext::PathType::Extension:
				return extensionPath;
			case PluginContext::PathType::Locale:
				return assetsPath / "locales";
			case PluginContext::PathType::Assets:
				return assetsPath;
			default:
				return modulePath;
			}
		}

		std::string PluginContext::SystemInfo()
		{
#if SC_MSVC
			std::stringstream result;

			HMODULE module = LoadLibrary(TEXT("ntdll.dll"));
			if (module) {
				typedef void (WINAPI* RtlGetVersion_FUNC) (OSVERSIONINFOEXW*);
				RtlGetVersion_FUNC func = (RtlGetVersion_FUNC)GetProcAddress(module, "RtlGetVersion");
				if (func == 0) {
					FreeLibrary(module);
					return "Unknown";
				}

				OSVERSIONINFOEXW info;
				ZeroMemory(&info, sizeof(info));

				info.dwOSVersionInfoSize = sizeof(info);
				func(&info);

				result << "Windows ";
				result << info.dwMajorVersion << "." << info.dwMinorVersion << "." << info.dwBuildNumber << " ";
			}
			else
			{
				return "Unknown";
			}

			FreeLibrary(module);

			return result.str();
#else
#error not implemented
#endif
		}
	}
}