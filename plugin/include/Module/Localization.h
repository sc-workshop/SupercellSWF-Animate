#pragma once

#include <string>
#include <fstream>
#include "Utils.h"

#include "JSON.hpp"

#include <filesystem>
namespace fs = std::filesystem;

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