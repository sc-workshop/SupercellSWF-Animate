#pragma once

#include "Module/PluginContext.h"
#include "Module/Localization.h"
#include "exception/GeneralRuntimeException.h"

namespace sc
{
	namespace Adobe
	{
		class PluginException : public GeneralRuntimeException
		{
		protected:
			std::u16string m_exceptionTitle;
			std::u16string m_exceptionDescription;

		public:
			template <class ... Args>
			PluginException(const char* exceptionId, Args ... args) : GeneralRuntimeException("PluginException")
			{
				PluginContext& context = PluginContext::Instance();

				m_exceptionTitle = context.locale.GetString(exceptionId, args...);
				m_exceptionDescription = context.locale.GetString(std::string(exceptionId) + "_description");
				m_message = Localization::ToUtf8(m_exceptionTitle);

				context.logger->error("Throwed PluginException");
				context.logger->error("	Message: {}", m_message);
			};

			PluginException(const std::u16string& reason);

			virtual const char16_t* Title() const;
			virtual const char16_t* Description() const;
		};
	}
}