#include "Module/PluginException.h"

namespace sc
{
	namespace Adobe
	{
		const char16_t* PluginException::title() const
		{
			return m_exceptionTitle.c_str();
		}

		const char16_t* PluginException::description() const
		{
			return m_exceptionDescription.c_str();
		}
	}
}