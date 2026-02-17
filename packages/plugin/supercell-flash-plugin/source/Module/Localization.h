#pragma once

#include "nlohmann/json.hpp"

#include <codecvt>
#include <filesystem>
#include <fmt/format.h>
#include <fmt/xchar.h>
#include <fstream>
#include <string>
using namespace nlohmann;

#include "animate/core/common/FCMPluginInterface.h"
#include "core/string/string_converter.h"

namespace sc::Adobe {
    class Localization {
    private:
        json m_locale;

    public:
        void Load(std::string LanguageCode);
        std::string GetUTF(const std::string& TID);

    public:
        template <class... Args>
        std::string GetU8String(const std::string& TID, Args... args) {
            json textValue = m_locale[TID];
            if (!textValue.is_string())
                return TID;

            std::string text = textValue;
            std::string formatted = fmt::format(fmt::runtime(text), args...);

            return formatted;
        }

        template <class... Args>
        std::u16string GetString(const std::string& TID, Args... args) {
            return wk::StringConverter::ToUTF16(GetU8String(TID, std::forward<Args>(args)...));
        }
    };
}
