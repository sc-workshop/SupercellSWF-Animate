#include "Ui/Components/ProgressBar.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

namespace sc {
	namespace Adobe {
		ProgressBar::ProgressBar(
			wxWindow* parent,
			const wxString& title,
			const wxString& defaulStatus,
			int range
		) : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(400, 75), wxTRANSPARENT_WINDOW | wxNO_BORDER)
		{

			m_label = new wxStaticText(this, wxID_ANY, title, wxPoint(0, 5));
			m_label->SetForegroundColour(wxColor(0xFFFFFF));

			m_progressBar = new wxGauge(this, wxID_ANY, range, wxPoint(10, 25), wxSize(325, 25));

			m_status = new wxStaticText(this, wxID_ANY, defaulStatus, wxPoint(10, 50));
			m_status->SetForegroundColour(wxColor(0xFFFFFF));
		}

		void ProgressBar::SetLabel(const wxString& label) {
			m_label->SetLabel(label);
		}

		void ProgressBar::SetStatus(const wxString& status) {
			m_status->SetLabel(status);
		}

		void ProgressBar::SetRange(int range) {
			m_progressBar->SetRange(range - 1);
		}

		void ProgressBar::SetProgress(int value) {
			m_progressBar->SetValue(value);
		}
	}
}