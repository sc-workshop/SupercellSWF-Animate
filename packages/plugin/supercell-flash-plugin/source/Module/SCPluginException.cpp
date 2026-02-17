#include "SCPluginException.h"

namespace sc::Adobe {
    SCPluginException::SCPluginException(const std::u16string& reason) {
        SCPlugin& context = SCPlugin::Instance();

        m_exceptionTitle = reason;
        m_message = FCM::Locale::ToUtf8(reason);

        context.logger->error("Called SCPluginException");
        context.logger->error("	Message: {}", m_message);
    };

    const std::u16string& SCPluginException::Title() const {
        return m_exceptionTitle;
    }

    const std::u16string& SCPluginException::Description() const {
        return m_exceptionDescription;
    }
}