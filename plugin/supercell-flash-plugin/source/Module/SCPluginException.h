#pragma once

#include "Module/Module.h"
#include "core/exception/exception.h"

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
			};

			SCPluginException(const std::u16string& reason);

			virtual const std::u16string& Title() const;
			virtual const std::u16string& Description() const;
		};
	}
}