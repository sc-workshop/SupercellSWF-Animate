#pragma once
#include <wx/wx.h>

namespace sc::Adobe {
    wxDEFINE_EVENT(WX_EVT_ERROR_RAISE, wxCommandEvent);

    class ErrorDialog : public wxDialog {
    public:
        ErrorDialog(wxWindow* parent, wxString errorMessage);
    };
}
