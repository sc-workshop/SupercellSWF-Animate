#include "Module/AppContext.h"

//#include "Module/Window.h"

namespace sc {
	namespace Adobe {
		AppContext::AppContext(FCM::PIFCMCallback callback, const FCM::PIFCMDictionary settings) {
			if (!callback) {
				throw std::exception("Failed to initialize app interface"); // TODO: replace all throw exception to Window::ThrowException
			}
			else {
				m_callback = callback;
			}
			falloc = getService<FCM::IFCMCalloc>(FCM::SRVCID_Core_Memory);

			locale.Load(languageCode());
			if (settings) {
				config = PublisherConfig::FromDict(settings);
			}
		};

		AppContext::~AppContext(){

		}

		std::string AppContext::languageCode() {
			FCM::AutoPtr<Application::Service::IApplicationService> appService =
				getService<Application::Service::IApplicationService>(Application::Service::APP_SERVICE);

			FCM::StringRep8 languageCodePtr;
			appService->GetLanguageCode(&languageCodePtr);

			std::string languageCode((const char*)languageCodePtr);

			falloc->Free(languageCodePtr);

			return languageCode;
		}

		void AppContext::trace(const char* fmt, ...) {
			FCM::AutoPtr<Application::Service::IOutputConsoleService> console =
				getService<Application::Service::IOutputConsoleService>(Application::Service::APP_OUTPUT_CONSOLE_SERVICE);

			va_list args;
			va_start(args, fmt);

			char buffer[1024];
			vsnprintf(buffer, 1024, fmt, args);

			string message(buffer);

			console->Trace((FCM::CStringRep16)Utils::ToUtf16(message + "\n").c_str());
		}
	}
}