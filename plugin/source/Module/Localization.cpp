#include "Module/Localization.h"
#include "Module/PluginContext.h"

#include <sstream>

namespace sc {
	namespace Adobe {
		void Localization::Load(std::string languageCode) {
			PluginContext& context = PluginContext::Instance();

			fs::path localesPath = PluginContext::CurrentPath(PluginContext::PathType::Locale);

			fs::path currentLocalePath = fs::path(localesPath / languageCode.append(".json"));

			if (!fs::exists(localesPath)) {
				currentLocalePath = fs::path(localesPath / "en_EN.json");
			}

			std::ifstream file(currentLocalePath);

			try {
				m_locale = json::parse(file, nullptr, true, true);
			}
			catch (const json::exception& exception)
			{
				context.print("Failed to load localization");
				context.print(exception.what());
			}
		};

		std::string Localization::GetUTF(const std::string& TID) {
			if (m_locale[TID].is_string()) {
				return m_locale[TID];
			}
			else {
				return TID;
			}
		}

		// std::u16string Localization::GetString(const std::string& TID, Args ... args) {
		// 	std::u16string result = GetString(TID, args);
		// }

		std::u16string Localization::ToUtf16(const std::string& string) {
			static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
			return convert.from_bytes(string);
		}

		std::string Localization::ToUtf8(const std::u16string& string) {
			std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
			return convert.to_bytes(string);
		}
	}
}