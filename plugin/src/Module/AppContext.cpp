#include "Module/AppContext.h"

namespace sc {
	namespace Adobe {
		AppContext::AppContext(
			FCM::PIFCMCallback callback,
			DOM::IFLADocument* _document,
			const FCM::PIFCMDictionary settings) : document(_document), m_callback(callback) {
			ASSERT(callback != nullptr);

			falloc = getService<FCM::IFCMCalloc>(FCM::SRVCID_Core_Memory);

			locale.Load(languageCode());
			config = Config::FromDict(settings);

			FCM::StringRep16 documentPathPtr;
			document->GetPath(&documentPathPtr);
			documentPath = fs::path((const char16_t*)documentPathPtr).remove_filename();
			falloc->Free(documentPathPtr);
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

		void AppContext::close() {
			if (window && window->ui && (!window->ui->readyToExit)) {
				window->ui->readyToExit = true;
				window->ui->Close();
			}
		}
	}
}