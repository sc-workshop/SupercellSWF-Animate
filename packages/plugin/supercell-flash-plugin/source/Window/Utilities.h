#pragma once

#include <wx/wx.h>
#include <string>
#include "core/string/string_converter.h"

namespace sc::Adobe {
    wxString CreateWxString(const std::u16string& str);
}
