// clang-format off
#include "Module/Module.h"
// clang-format on

#include "Window/Components/ErrorDialog.h"

namespace sc::Adobe {
    ErrorDialog::ErrorDialog(wxWindow* parent, wxString errorMessage) :
        wxDialog(parent, wxID_ANY, "Error", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP) {
        SetBackgroundColour(wxColor(0x333333));

        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        wxStaticText* text = new wxStaticText(this, wxID_ANY, errorMessage);
        text->SetForegroundColour(wxColor(0xFFFFFF));

        sizer->Add(text, 0, wxALL | wxALIGN_CENTER, 10);

        wxButton* okButton = new wxButton(this, wxID_OK | wxICON_ERROR, "OK");
        sizer->Add(okButton, 0, wxALL | wxALIGN_CENTER, 10);

        SetSizerAndFit(sizer);
        Centre();
    }
}
