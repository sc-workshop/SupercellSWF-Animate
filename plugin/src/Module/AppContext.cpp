#include "Module/AppContext.h"

namespace sc {
	namespace Adobe {
		string AppContext::languageCode() {
			AutoPtr<IApplicationService> appService = getService<IApplicationService>(APP_SERVICE);

			StringRep8 languageCodePtr;
			appService->GetLanguageCode(&languageCodePtr);

			string languageCode((const char*)languageCodePtr);

			falloc->Free(languageCodePtr);

			return languageCode;
		}

		void AppContext::trace(const char* fmt, ...) {
			AutoPtr<IOutputConsoleService> console = getService<IOutputConsoleService>(APP_OUTPUT_CONSOLE_SERVICE);

			va_list args;
			va_start(args, fmt);

			char buffer[1024];
			vsnprintf(buffer, 1024, fmt, args);

			string message(buffer);

			console->Trace((CStringRep16)Utils::ToUtf16(message + "\n").c_str());
		}
	}
}