#include "Module/Module.h"
#include "Window/PluginWindow.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

namespace sc::Adobe {
	wxBEGIN_EVENT_TABLE(PluginWindow, wxDialog)
		EVT_CLOSE(PluginWindow::OnClose)
		SC_EVT_CREATE_PROGRESS(wxID_ANY, PluginWindow::OnProgressCreate)
		SC_EVT_DESTROY_PROGRESS(wxID_ANY, PluginWindow::OnProgressDestroy)
    wxEND_EVENT_TABLE();

	PluginWindow::PluginWindow(wxString title)
		: wxDialog(
			nullptr,
			wxID_ANY,
			title,
			wxDefaultPosition,
			wxDefaultSize,
            wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP | wxRESIZE_BORDER)
	{
		SCPlugin& context = SCPlugin::Instance();
		context.logger->info("PluginWindow initialized. Creating elements...");
        
		Center();
		SetBackgroundColour(wxColor(0x333333));

		panelSizer = new wxBoxSizer(wxVERTICAL);
		contentSizer = new wxBoxSizer(wxVERTICAL);

		panel = new wxPanel(this, wxID_ANY);
		panelSizer->Add(panel, 1, wxEXPAND);
		panel->SetSizer(contentSizer);
		SetSizer(panelSizer);

		context.logger->info("Elements successfully created");

		Bind(WX_EVT_ERROR_RAISE, [&](wxCommandEvent& event) {
			ErrorDialog* dialog = new ErrorDialog(m_parent, event.GetString());
			dialog->ShowModal();
			readyToExit = true;
			Close(true);
			wxTheApp->ExitMainLoop();
		});
        
        Bind(wxEVT_SHOW, [&](wxShowEvent& event){
            if (event.IsShown()) {
                m_ready = true;
                m_window_cv.notify_all();
            }
        });
	}

    void PluginWindow::Wait() {
        std::unique_lock<std::mutex> guard(m_window_mut);
        m_window_cv.wait(guard, [&]{ return m_ready; });
    }

	StatusComponent* PluginWindow::CreateStatusBarComponent(
		const std::u16string& title,
		const std::u16string& defaultLabel,
		int range
	) {
		StatusComponent* result = nullptr;

		PluginCreateProgressEvent* event = new PluginCreateProgressEvent(title, defaultLabel, range, result);
		event->SetEventObject(this);
		GetEventHandler()->QueueEvent(event);

		// await result
		std::unique_lock<std::mutex> guard(m_window_mut);
		m_window_cv.wait(guard, [&result] {return result != nullptr; });

		m_elements_counter++;
		return result;
	}

	void PluginWindow::DestroyStatusBar(StatusComponent* bar)
	{
		bool destroyed = false;
		PluginDestroyProgressEvent* event = new PluginDestroyProgressEvent(bar, &destroyed);
		event->SetEventObject(this);
		GetEventHandler()->QueueEvent(event);

		// await result
		std::unique_lock<std::mutex> guard(m_window_mut);
		m_window_cv.wait(guard, [&destroyed] {return destroyed; });

		m_elements_counter--;
	}

	void PluginWindow::ThrowException(wxString what) {
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
		readyToExit = readyToExit || m_elements_counter == 0;

		if (!aboutToExit && !readyToExit) {
			if (wxMessageBox("Are you sure you want to exit?", "Confirmation", wxYES_NO | wxICON_QUESTION) == wxYES) {
				aboutToExit = true;
			}
			else {
				event.Veto();
			}
		}

		if (readyToExit) {
            EndModal(wxID_OK);
		}
		else {
			event.Veto();
		}
	}

	void PluginWindow::OnProgressCreate(PluginCreateProgressEvent& event)
	{
		Freeze();
		StatusComponent* progressBar = new StatusComponent(panel, event.title, event.status, event.range);
		contentSizer->Add(progressBar);
		ScaleByContent();
		Thaw();

		event.result = progressBar;

		m_window_cv.notify_all();
	}

	void PluginWindow::OnProgressDestroy(PluginDestroyProgressEvent& event)
	{
		event.component->Destroy();
		ScaleByContent();

		*event.destroyed = true;

		m_window_cv.notify_all();
	}
}
