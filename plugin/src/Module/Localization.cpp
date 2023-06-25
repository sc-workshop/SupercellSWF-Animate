#include "Module/Localization.h"

#include <sstream>

namespace sc {
	namespace Adobe {
		void Localization::Load(std::string languageCode) {
			fs::path localePath = Utils::CurrentPath() / ("../res/locales/" + languageCode + ".json");

			if (!fs::exists(localePath)) {
				localePath = Utils::CurrentPath() / "../res/locales/" DefaultLanguageCode ".json";
			}

			std::ifstream file(localePath.make_preferred());
			m_locale = json::parse(file);
		};

		std::string Localization::GetString(std::string TID) {
			if (m_locale[TID].is_string()) {
				return m_locale[TID];
			}
			else {
				return TID;
			}
		}

		std::u16string Localization::Get(std::string TID) {
			return Utils::ToUtf16(GetString(TID));
		}
	}
}