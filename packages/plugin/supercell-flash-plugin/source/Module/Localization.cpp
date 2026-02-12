#include "Module/Localization.h"
#include "Module/Module.h"
#include <sstream>

#include "AnimateCore.h"

namespace fs = std::filesystem;

namespace sc {
	namespace Adobe {
		void Localization::Load(std::string LanguageCode) {
			SCPlugin& context = SCPlugin::Instance();

			fs::path localesPath = FCM::PluginModule::CurrentPath(FCM::PluginModule::PathType::Locale);
			context.logger->info("Locales storage: {}", localesPath.string());

			fs::path currentLocalePath = fs::path(localesPath / LanguageCode.append(".json"));

			if (!fs::exists(currentLocalePath)) {
				currentLocalePath = fs::path(localesPath / "en_US.json");
			}

			context.logger->info("Active locale file: {}", currentLocalePath.string());
			std::ifstream file(currentLocalePath);

			try {
				m_locale = json::parse(file, nullptr, true, true);
			}
			catch (const json::exception& exception)
			{
				context.logger->error("Failed to parse locale file: {}", exception.what());
				context.Trace("Failed to load localization");
				context.Trace(exception.what());
			}
		};

		std::string Localization::GetUTF(const std::string& TID) {
			auto value = m_locale[TID];
			if (value.is_string()) {
				return value;
			}
			else {
				return TID;
			}
		}
	}
}
