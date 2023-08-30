#pragma once

#include <wx/wx.h>
#include <wx/stattext.h>
#include <wx/gauge.h>

namespace sc {
	namespace Adobe {
		class StatusComponent : public wxPanel
		{
			wxGauge* m_progressBar = nullptr;
			wxStaticText* m_label = nullptr;
			wxStaticText* m_status = nullptr;

		public:
			StatusComponent(
				wxWindow* parent,
				const wxString& title = "",
				const wxString& defaultLabel = "",
				int range = 99 // Equals to 100 percent
			);

		public:
			void SetLabel(const wxString& label);
			void SetStatus(const wxString& status);
			void SetRange(int range);
			void SetProgress(int value);
		};
	}
}