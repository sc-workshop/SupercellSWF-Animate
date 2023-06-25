#include "Ui/Window.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

namespace sc {
	namespace Adobe {
		wxBEGIN_EVENT_TABLE(PluginUIWindow, wxFrame)
			EVT_CLOSE(PluginUIWindow::OnClose)
		wxEND_EVENT_TABLE()

		PluginUIWindow::PluginUIWindow(wxString title)
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

		PluginUIWindow::~PluginUIWindow()
		{

		}

		void PluginUIWindow::SetLabel(const wxString& str) {
			m_defProgressBar->SetLabel(str);
		}

		void PluginUIWindow::SetLabel(const std::u16string& str) {
			m_defProgressBar->SetLabel( wxString((const wchar_t*)str.c_str() ));
		}

		void PluginUIWindow::SetStatus(const wxString& str) {
			m_defProgressBar->SetStatus(str);
		}

		void PluginUIWindow::SetStatus(const std::u16string& str) {
			m_defProgressBar->SetStatus(wxString((const wchar_t*)str.c_str()));
		}

		void PluginUIWindow::SetProgress(uint8_t stage)
		{
			m_defProgressBar->SetProgress(stage);
		}

		void PluginUIWindow::SetAdditionalStatusBarCount(uint8_t count) {
			m_progressBars.clear();

			for (uint8_t i = 0; count > i; i++) {
				ProgressBar* progressBar = CreateStatusBar();
				progressBar->Hide();
				m_progressBars.push_back({false, progressBar });
			}

			ScaleByContent();
		}

		ProgressBar* PluginUIWindow::GetAvailableProgressBar() {
			for (std::pair<bool, ProgressBar*> obj : m_progressBars) {
				if (obj.first) continue;
				obj.second->Show();
				ScaleByContent();
				return obj.second;
			}

			return nullptr;
		}

		void PluginUIWindow::DestroyProgressBar(ProgressBar* ptr) {
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

		ProgressBar* PluginUIWindow::CreateStatusBar(
			const wxString& title,
			const wxString& defaultLabel, 
			int range
		){
			ProgressBar* progressBar = new ProgressBar(panel, title, defaultLabel, range);
			contentSizer->Add(progressBar);
			ScaleByContent();

			return progressBar;
		}

		void PluginUIWindow::ThrowException(const wxString& what) {
			wxCommandEvent* exceptionEvent = new wxCommandEvent(WX_EVT_ERROR_RAISE);
			exceptionEvent->SetString(what);
			exceptionEvent->SetEventObject(this);
			wxQueueEvent(this, exceptionEvent);
		}

		void PluginUIWindow::ScaleByContent() {
			panel->Fit();
			panelSizer->Fit(this);
			Layout();
		}

		void PluginUIWindow::OnClose(wxCloseEvent& event) {
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