#pragma once

#include <wx/wx.h>

namespace sc {
    namespace Adobe {
        wxDEFINE_EVENT(WX_EVT_ERROR_RAISE, wxCommandEvent);

        class ErrorDialog : public wxDialog
        {
        public:
            ErrorDialog(wxWindow* parent, const wxString& errorMessage);
        };
    }
}

