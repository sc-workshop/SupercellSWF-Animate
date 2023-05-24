#pragma once

#include <string>
#include <filesystem>
#include <fstream>

#include <libjson.h>

namespace fs = std::filesystem;
using namespace std;

#define DefaultLanguageCode "en_EN"

namespace sc {
	namespace Adobe {
		class LocaleInterface {
		private:
			JSONNode locale;

		public:
			LocaleInterface() { };

			void Load(fs::path baseFolder, string languageCode) {
				fs::path localePath = fs::path(baseFolder) / ("../../../locales/" + languageCode + ".json");

				if (!fs::exists(localePath)) {
					localePath = fs::path(baseFolder) / "../../../locales/" DefaultLanguageCode ".json";
				}

				ifstream file(localePath);
				if (!file) return;
				size_t fileSize = 0;

				file.seekg(0, file.end);
				fileSize = file.tellg();
				file.seekg(0, file.beg);

				std::string buffer(fileSize, ' ');

				file.read(buffer.data(), fileSize);
				file.close();

				locale = libjson::parse(buffer);
			}

			string Get(string TID) {
				string result(TID);

				for (JSONNode node : locale) {
					if (node.name() == TID) {
						return node.as_string();
					}
				}

				return TID;
			}
		};

		LocaleInterface Locale;
	}
}