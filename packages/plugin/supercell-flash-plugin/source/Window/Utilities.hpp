#pragma once

#include <wx/wx.h>
#include <string>
#include "core/string/string_converter.h"

namespace sc::Adobe {
    static wxString CreateWxString(const std::u16string& str) {
#if defined(_WINDOWS)
        // On Windows wchar is already 2 bytes long, so we can just interprete pointer
        return wxString((const wchar_t*)str.c_str());
#elif defined(__APPLE__)
        // On apple, wchar is 4 bytes long, so we need to convert u16string to utf32
        std::u32string converted = wk::StringConverter::ToUTF32(str);
        return wxString((const wchar_t*)converted.c_str(), converted.size());
#endif
    }
}
