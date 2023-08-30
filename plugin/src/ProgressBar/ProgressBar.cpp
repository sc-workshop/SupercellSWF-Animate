#include "ProgressBar/ProgressBar.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

namespace sc {
	namespace Adobe {
		wxBEGIN_EVENT_TABLE(ProgressBarWindow, wxFrame)
			EVT_CLOSE(ProgressBarWindow::OnClose)
			wxEND_EVENT_TABLE();

		ProgressBarWindow::ProgressBarWindow(wxString title)
			: wxFrame(
				nullptr,
				wxID_ANY,
				title,
				wxDefaultPosition,
				wxDefaultSize,
				wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX))
		{
			Center();
			SetBackgroundColour(wxColor(0x333333));

			panelSizer = new wxBoxSizer(wxVERTICAL);
			contentSizer = new wxBoxSizer(wxVERTICAL);

			panel = new wxPanel(this, wxID_ANY);
			panelSizer->Add(panel, 1, wxEXPAND);
			panel->SetSizer(contentSizer);
			SetSizer(panelSizer);

			m_defProgressBar = CreateStatusBar("", "", 99);

			Bind(WX_EVT_ERROR_RAISE, [&](wxCommandEvent& event) {
				ErrorDialog* dialog = new ErrorDialog(m_parent, event.GetString().wchar_str());
				dialog->ShowModal();
				dialog->Destroy();
				readyToExit = true;
				Close(true);
				});
		}

		void ProgressBarWindow::SetLabel(const wxString& str) {
			m_defProgressBar->SetLabel(str);
		}

		void ProgressBarWindow::SetLabel(const std::u16string& str) {
			m_defProgressBar->SetLabel(wxString((const wchar_t*)str.c_str()));
		}

		void ProgressBarWindow::SetStatus(const wxString& str) {
			m_defProgressBar->SetStatus(str);
		}

		void ProgressBarWindow::SetStatus(const std::u16string& str) {
			m_defProgressBar->SetStatus(wxString((const wchar_t*)str.c_str()));
		}

		void ProgressBarWindow::SetProgress(uint8_t stage)
		{
			m_defProgressBar->SetProgress(stage);
		}

		void ProgressBarWindow::SetAdditionalStatusBarCount(uint8_t count) {
			m_progressBars.clear();

			for (uint8_t i = 0; count > i; i++) {
				StatusComponent* progressBar = CreateStatusBar();
				progressBar->Hide();
				m_progressBars.push_back({ false, progressBar });
			}

			ScaleByContent();
		}

		StatusComponent* ProgressBarWindow::GetAvailableProgressBar() {
			for (std::pair<bool, StatusComponent*> obj : m_progressBars) {
				if (obj.first) continue;
				obj.second->Show();
				ScaleByContent();
				return obj.second;
			}

			return nullptr;
		}

		void ProgressBarWindow::DestroyProgressBar(StatusComponent* ptr) {
			for (uint8_t i = 0; m_progressBars.size() > i; i++) {
				if (m_progressBars[i].second == ptr) {
					ptr->Hide();
					ptr->SetLabel("");
					ptr->SetStatus("");
					ptr->SetRange(99);
					ScaleByContent();

					m_progressBars[i].first = false;
				}
			}
		}

		StatusComponent* ProgressBarWindow::CreateStatusBar(
			const wxString& title,
			const wxString& defaultLabel,
			int range
		) {
			StatusComponent* progressBar = new StatusComponent(panel, title, defaultLabel, range);
			contentSizer->Add(progressBar);
			ScaleByContent();

			return progressBar;
		}

		void ProgressBarWindow::ThrowException(const wxString& what) {
			wxCommandEvent* exceptionEvent = new wxCommandEvent(WX_EVT_ERROR_RAISE);
			exceptionEvent->SetString(what);
			exceptionEvent->SetEventObject(this);
			wxQueueEvent(this, exceptionEvent);
		}

		void ProgressBarWindow::ScaleByContent() {
			panel->Fit();
			panelSizer->Fit(this);
			Layout();
		}

		void ProgressBarWindow::OnClose(wxCloseEvent& event) {
			if (!aboutToExit && !readyToExit) {
				if (wxMessageBox("Are you sure you want to exit?", "Confirmation", wxYES_NO | wxICON_QUESTION) == wxNO) {
					event.Veto();
				}
				else {
					aboutToExit = true;
				}
			}

			if (readyToExit) {
				event.Skip();
			}
			else {
				event.Veto();
			}
		}
	}
}