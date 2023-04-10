#pragma once

#include "FCMTypes.h"
#include "FCMPluginInterface.h"
#include "Utils/DOMTypes.h"
#include "IFCMStringUtils.h"
#include "Application/Service/IOutputConsoleService.h"
#include "Application/Service/IApplicationService.h"
#include "ApplicationFCMPublicIDs.h"

#include "Utils.h"

#include <iostream>
#include <string>
#include <stdarg.h>

struct Console {
	Console() {};
	~Console() {};

	void Init(const std::string loggerName, const FCM::PIFCMCallback callback) {
		m_name = loggerName;
		m_callback = callback;
	}

	void log(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		trace(fmt, args);
		va_end(args);
	}

	void debugLog(const char* fmt, ...)
	{
#ifdef _DEBUG
		va_list args;
		va_start(args, fmt);
		trace(fmt, args);
		va_end(args);
#endif
	}

private:

	void trace(const char* fmt, va_list args) {
		if (m_name.empty() || !m_callback) {
			return;
		}
		FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
		FCM::AutoPtr<Application::Service::IOutputConsoleService> outputConsoleService;
		FCM::Result tempRes = m_callback->GetService(Application::Service::APP_OUTPUT_CONSOLE_SERVICE, pUnk.m_Ptr);
		outputConsoleService = pUnk;
		pUnk.Reset();

		if (outputConsoleService)
		{
			char buffer[1024];
			vsnprintf(buffer, 1024, fmt, args);

			FCM::AutoPtr<FCM::IFCMCalloc> calloc = SupercellSWF::Utils::GetCallocService(m_callback);
			ASSERT(calloc.m_Ptr != NULL);

			std::string string = "[" + m_name + "] " + std::string(buffer) + "\n";
			FCM::StringRep16 outputString = SupercellSWF::Utils::ToString16(string, m_callback);
			outputConsoleService->Trace(outputString);
			calloc->Free(outputString);
		}
	}

private:
	std::string m_name;
	FCM::PIFCMCallback m_callback;
};