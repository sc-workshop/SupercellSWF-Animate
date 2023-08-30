#include "Module/Context.h"

namespace sc {
	namespace Adobe {
		Context::Context(
			FCM::PIFCMCallback callback,
			DOM::IFLADocument* _document,
			const FCM::PIFCMDictionary settings) : document(_document), m_callback(callback) {
			ASSERT(callback != nullptr);

			falloc = getService<FCM::IFCMCalloc>(FCM::SRVCID_Core_Memory);

			locale.Load(languageCode());
			config = Config::FromDict(settings);

			fs::path documentPath;
			{
				FCM::StringRep16 documentPathPtr;
				document->GetPath(&documentPathPtr);
				if (documentPathPtr)
				{
					documentPath = fs::path((const char16_t*)documentPathPtr);
					falloc->Free(documentPathPtr);
				}
			}

			if (config.output.empty())
			{
				if (!documentPath.empty())
				{
					config.output = documentPath;
				}
			}
			else if (config.output.is_relative())
			{
				if (!documentPath.empty())
				{
					config.output = (documentPath.parent_path() / config.output).make_preferred();
				}
			}

			if (config.exportToExternal && !config.exportToExternalPath.empty())
			{
				if (!documentPath.empty())
				{
					config.exportToExternalPath = (documentPath.parent_path() / config.exportToExternalPath).make_preferred();
				}
			}

			if (config.output.has_extension())
			{
				fs::path outputExt = config.output.extension();
				if (outputExt.compare(".xfl") == 0)
				{
					config.output = config.output.parent_path();
				}
				else
				{
					config.output.replace_extension();
				}
			}
		};

		std::string Context::languageCode() {
			FCM::AutoPtr<Application::Service::IApplicationService> appService =
				getService<Application::Service::IApplicationService>(Application::Service::APP_SERVICE);

			FCM::StringRep8 languageCodePtr;
			appService->GetLanguageCode(&languageCodePtr);

			std::string languageCode((const char*)languageCodePtr);

			falloc->Free(languageCodePtr);

			return languageCode;
		}

		void Context::trace(const char* fmt, ...) {
			FCM::AutoPtr<Application::Service::IOutputConsoleService> console =
				getService<Application::Service::IOutputConsoleService>(Application::Service::APP_OUTPUT_CONSOLE_SERVICE);

			va_list args;
			va_start(args, fmt);

			char buffer[1024];
			vsnprintf(buffer, 1024, fmt, args);

			string message(buffer);

			console->Trace((FCM::CStringRep16)Utils::ToUtf16(message + "\n").c_str());
		}

		void Context::close() {
			if (progressBar && progressBar->window && (!progressBar->window->readyToExit)) {
				progressBar->window->readyToExit = true;
				progressBar->window->Close();
			}
		}
	}
}