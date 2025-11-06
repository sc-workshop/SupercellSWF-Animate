#include "Window/PluginWindow.h"

namespace sc {
	namespace Adobe {
		StatusComponent::StatusComponent(
			wxWindow* parent,
			const std::u16string& title,
			const std::u16string& defaulStatus,
			int range
		) : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(400, 75), wxTRANSPARENT_WINDOW | wxNO_BORDER)
		{
			SetBackgroundStyle(wxBG_STYLE_PAINT);
			Bind(wxEVT_ERASE_BACKGROUND, [](wxEraseEvent&) {});

			m_label = new wxStaticText(this, wxID_ANY, (const wchar_t*)title.c_str(), wxPoint(0, 5));
			m_label->SetForegroundColour(wxColor(0xFFFFFF));

			m_progressBar = new ui::ProgressBar(this);
			m_progressBar->SetRange(range);
			m_progressBar->SetPosition(wxPoint(10, 25));
			m_progressBar->SetSize(wxSize(325, 20));

			m_status = new wxStaticText(this, wxID_ANY, (const wchar_t*)defaulStatus.c_str(), wxPoint(10, 50));
			m_status->SetForegroundColour(wxColor(0xFFFFFF));
		}

		void StatusComponent::SetStatusLabel(const std::u16string& label) {
			m_label->SetLabel((const wchar_t*)label.c_str());
		}

		void StatusComponent::SetStatus(const std::u16string& status) {
			m_status->SetLabel((const wchar_t*)status.c_str());
		}

		void StatusComponent::SetRange(int range) {
			m_progressBar->SetRange(range);
		}

		void StatusComponent::SetProgress(int value) {
			m_progressBar->SetValue(value);
		}
	}
}