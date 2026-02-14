#pragma once

#include "Module/Module.h"
#include "core/exception/exception.h"

#if defined(__APPLE__) && WK_DEBUG
#include <execinfo.h>
#endif

namespace sc
{
	namespace Adobe
	{
		class SCPluginException : public wk::Exception
		{
		protected:
			std::u16string m_exceptionTitle;
			std::u16string m_exceptionDescription;

		public:
			template <class ... Args>
			SCPluginException(const char* exceptionId, Args ... args)
			{
				SCPlugin& context = SCPlugin::Instance();

				m_exceptionTitle = context.locale.GetString(exceptionId, args...);
				m_exceptionDescription = context.locale.GetString(std::string(exceptionId) + "_description");
				m_message = FCM::Locale::ToUtf8(m_exceptionTitle);

				context.logger->error("Called SCPluginException");
				context.logger->error("	Message: {}", m_message);
#if defined(__APPLE__) && WK_DEBUG
                void* callstack[128];
                int frames = backtrace(callstack, 128);

                char** symbols = backtrace_symbols(callstack, frames);

                for (int i = 0; i < frames; ++i)
                    context.logger->error("    Stack: {}", symbols[i]);

                free(symbols);
#endif
			};

			SCPluginException(const std::u16string& reason);

			virtual const std::u16string& Title() const;
			virtual const std::u16string& Description() const;
		};
	}
}
