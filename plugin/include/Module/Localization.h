#pragma once

#include <string>
#include <fstream>

#include <filesystem>
namespace fs = std::filesystem;

#include "Utils.h"

#include "json.hpp"
using namespace nlohmann;

#define DefaultLanguageCode "en_EN"
#define wxStringU16(str) wxString((const wchar_t*)str.c_str())

namespace sc {
	namespace Adobe {
		class Localization {
		private:
			json m_locale;

		public:
			void Load(std::string languageCode);

			std::string GetString(std::string TID);

			std::u16string Get(std::string TID);
		};
	}
}