#pragma once

#include "core/string/string_converter.h"

#include <string>
#include <wx/wx.h>

namespace sc::Adobe {
    wxString CreateWxString(const std::u16string& str);
}
