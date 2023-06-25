#include "Ui/Components/ErrorDialog.h"

namespace sc {
	namespace Adobe {
        ErrorDialog::ErrorDialog(wxWindow* parent, const wxString& errorMessage)
            : wxDialog(parent, wxID_ANY, "Error")
        {
            SetBackgroundColour(wxColor(0x333333));

            wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            wxStaticText* text = new wxStaticText(this, wxID_ANY, errorMessage);
            text->SetForegroundColour(wxColor(0xFFFFFF));

            sizer->Add(text, 0, wxALL | wxALIGN_CENTER, 10);

            wxButton* okButton = new wxButton(this, wxID_OK, "ÎÊ");
            sizer->Add(okButton, 0, wxALL | wxALIGN_CENTER, 10);

            SetSizerAndFit(sizer);
            Centre();
        }
	}
}