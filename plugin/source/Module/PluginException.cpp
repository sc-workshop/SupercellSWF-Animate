#include "Module/PluginException.h"

#include "Module/PluginContext.h"

namespace sc
{
	namespace Adobe
	{
		PluginException::PluginException(const std::u16string& reason) : GeneralRuntimeException("PluginException")
		{
			PluginContext& context = PluginContext::Instance();

			m_exceptionTitle = reason;
			m_message = Localization::ToUtf8(reason);

			context.logger->error("Throwed PluginException");
			context.logger->error("	Message: {}", m_message);
		};

		const std::u16string& PluginException::Title() const
		{
			return m_exceptionTitle;
		}

		const std::u16string& PluginException::Description() const
		{
			return m_exceptionDescription;
		}
	}
}