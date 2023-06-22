#pragma once

#include <string>
#include <fstream>
#include "Utils.h"

#include "JSON.hpp"

#include <filesystem>
namespace fs = std::filesystem;

using namespace nlohmann;

#define DefaultLanguageCode "en_EN"

namespace sc {
	namespace Adobe {
		class LocaleInterface {
		private:
			json locale;

		public:
			void Load(std::string languageCode) { 
				/*fs::path localePath = Utils::CurrentPath() / ("../../../locales/" + languageCode + ".json");

				if (!fs::exists(localePath)) {
					localePath = Utils::CurrentPath() / "../../../locales/" DefaultLanguageCode ".json";
				}

				ifstream file(localePath);
				locale = json::parse(file);
				file.close();*/
			};

			std::u16string Get(std::string TID) {
				if (locale[TID]) {
					return locale[TID];
				}
				else {
					return Utils::ToUtf16(TID);
				}
			}
		};
	}
}