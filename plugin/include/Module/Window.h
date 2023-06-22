#pragma once

#include <wx/wx.h>
#include <wx/gauge.h>

namespace sc {
	namespace Adobe {
		class Window : public wxFrame
		{
		public:
			Window(const wxString& title) : wxFrame(NULL, wxID_ANY, "Publisher")
			{
				wxPanel* panel = new wxPanel(this, wxID_ANY);
				progressBar = new wxGauge(panel, wxID_ANY, 100, wxPoint(10, 10), wxSize(250, 25));
			}

			void SetProgress(int value)
			{
				progressBar->SetValue(value);
			}

		private:
			wxGauge* progressBar;
		};
	}
}
