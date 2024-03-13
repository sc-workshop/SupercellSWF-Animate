#include "Module/PluginContext.h"

#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#endif

namespace sc
{
	namespace Adobe
	{
		void PluginContext::UpdateCallback(FCM::PIFCMCallback active_callback)
		{
			callback = active_callback;
			falloc = getService<FCM::IFCMCalloc>(FCM::SRVCID_Core_Memory);
			console = getService<Application::Service::IOutputConsoleService>(Application::Service::APP_OUTPUT_CONSOLE_SERVICE);
			appService = getService<Application::Service::IApplicationService>(Application::Service::APP_SERVICE);
			locale.Load(languageCode());
		}

		std::string PluginContext::languageCode() {
			FCM::StringRep8 languageCodePtr;
			appService->GetLanguageCode(&languageCodePtr);

			std::string languageCode((const char*)languageCodePtr);

			falloc->Free(languageCodePtr);

			return languageCode;
		}

		bool& PluginContext::initializeWindow()
		{
			m_app = new PluginWindowApp();
			wxApp::SetInstance(m_app);

			return m_app->isInited;
		}

		void PluginContext::destroyWindow()
		{
			if (m_app)
			{
				m_app->window->aboutToExit = true;
				m_app->window->readyToExit = true;
				m_app->window->Close(true);

				m_app = nullptr;
			}
		}

		PluginWindow* PluginContext::window()
		{
			return m_app->window;
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
	}
}