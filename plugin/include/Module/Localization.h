#pragma once

#include <string>
#include <fstream>
#include <codecvt>

#include <filesystem>
namespace fs = std::filesystem;

#include "nlohmann/json.hpp"
using namespace nlohmann;

#define SC_wxStringU16(str) wxString((const wchar_t*)str.c_str())

namespace sc {
	namespace Adobe {
		class Localization {
		private:
			json m_locale;

		public:
			void Load(std::string LanguageCode);
			std::string GetUTF(const std::string& TID);

		public:
			template <class ... Args>
			std::u16string GetString(const std::string& TID, Args ... args)
			{
				auto text = m_locale.at(TID);
				if (!text.is_string()) return Localization::ToUtf16(TID);

				std::u16string format = Localization::ToUtf16(text);

				const size_t bufferSize = 1024;
				wchar_t buffer[bufferSize] = { 0 };
				std::swprintf(buffer, bufferSize, reinterpret_cast<const wchar_t*>(format.c_str()), args...);

				return std::u16string(reinterpret_cast<const char16_t*>(buffer));
			}

		public:
			static std::u16string ToUtf16(const std::string& string);

			static std::string ToUtf8(const std::u16string& string);
		};
	}
}