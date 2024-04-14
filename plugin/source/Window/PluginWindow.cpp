#include "Window/PluginWindow.h"
#include "Module/PluginContext.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

namespace sc {
	namespace Adobe {
		wxBEGIN_EVENT_TABLE(PluginWindow, wxFrame)
			EVT_CLOSE(PluginWindow::OnClose)
			EVT_MYFOO(wxID_ANY, PluginWindow::OnProgressCreate)
			wxEND_EVENT_TABLE();

		PluginWindow::PluginWindow(const std::u16string& title)
			: wxFrame(
				nullptr,
				wxID_ANY,
				(const wchar_t*)title.c_str(),
				wxDefaultPosition,
				wxDefaultSize,
				wxDEFAULT_FRAME_STYLE | wxSTAY_ON_TOP & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX))
		{
			Center();
			SetBackgroundColour(wxColor(0x333333));

			panelSizer = new wxBoxSizer(wxVERTICAL);
			contentSizer = new wxBoxSizer(wxVERTICAL);

			panel = new wxPanel(this, wxID_ANY);
			panelSizer->Add(panel, 1, wxEXPAND);
			panel->SetSizer(contentSizer);
			SetSizer(panelSizer);

			Bind(WX_EVT_ERROR_RAISE, [&](wxCommandEvent& event) {
				ErrorDialog* dialog = new ErrorDialog(m_parent, event.GetString());
				dialog->ShowModal();
				dialog->Destroy();
				readyToExit = true;
				});
		}

		StatusComponent* PluginWindow::CreateStatusBar(
			const std::u16string& title,
			const std::u16string& defaultLabel,
			int range
		) {
			StatusComponent* result = nullptr;

			PluginCreateProgressEvent* event = new PluginCreateProgressEvent(title, defaultLabel, range, result);

			GetEventHandler()->QueueEvent(event);

			// await result
			while (!result);

			return result;
		}

		void PluginWindow::ThrowException(const wxString& what) {
			wxCommandEvent* exceptionEvent = new wxCommandEvent(WX_EVT_ERROR_RAISE);
			exceptionEvent->SetString(what);
			exceptionEvent->SetEventObject(this);

			GetEventHandler()->QueueEvent(exceptionEvent);
		}

		void PluginWindow::ScaleByContent() {
			panel->Fit();
			panelSizer->Fit(this);
			Layout();
		}

		void PluginWindow::OnClose(wxCloseEvent& event) {
			if (!aboutToExit && !readyToExit) {
				if (wxMessageBox("Are you sure you want to exit?", "Confirmation", wxYES_NO | wxICON_QUESTION) == wxYES) {
					aboutToExit = true;
				}
				else {
					event.Veto();
				}
			}

			if (readyToExit) {
				event.Skip();
			}
			else {
				event.Veto();
			}
		}

		void PluginWindow::OnProgressCreate(PluginCreateProgressEvent& event)
		{
			StatusComponent* progressBar = new StatusComponent(panel, event.title, event.status, event.range);
			contentSizer->Add(progressBar);
			ScaleByContent();

			event.result = progressBar;
		}
	}
}