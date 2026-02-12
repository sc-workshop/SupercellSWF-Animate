#pragma once

#include <string>
#include <fstream>
#include <codecvt>

#include <filesystem>

#include "nlohmann/json.hpp"
using namespace nlohmann;

#include "animate/core/common/FCMPluginInterface.h"
#include "core/string/string_converter.h"

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
                json textValue = m_locale[TID];
				if (!textValue.is_string()) return wk::StringConverter::ToUTF16(TID);
                
                std::string text = textValue;
				std::u16string format = wk::StringConverter::ToUTF16(text);

				return Localization::Format(format, args...);
			}
#if defined(_WINDOWS)
			template <class ... Args>
			static std::u16string Format(const std::u16string& message, Args ... args)
			{
				const size_t bufferSize = 1024;
				wchar_t buffer[bufferSize] = { 0 };
				std::swprintf(buffer, bufferSize, reinterpret_cast<const wchar_t*>(message.c_str()), args...);

				return std::u16string(reinterpret_cast<const char16_t*>(buffer));
			}
#elif defined(__APPLE__)
            template <class ... Args>
            static std::u16string Format(const std::u16string& message, Args ... args)
            {
                std::u32string converted = wk::StringConverter::ToUTF32(message);
                
                const size_t bufferSize = 1024;
                wchar_t buffer[bufferSize] = { 0 };
                std::swprintf(buffer, bufferSize, reinterpret_cast<const wchar_t*>(converted.c_str()), args...);

                return wk::StringConverter::ToUTF16(std::u32string(reinterpret_cast<const char32_t*>(buffer)));
            }
#endif
		};
	}
}
