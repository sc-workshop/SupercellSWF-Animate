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
				const std::u16string& title = u"",
				const std::u16string& defaultLabel = u"",
				int range = 99 // Equals to 100 percent
			);

		public:
			void SetStatusLabel(const std::u16string& label);
			void SetStatus(const std::u16string& status);
			void SetRange(int range);
			void SetProgress(int value);
		};
	}
}