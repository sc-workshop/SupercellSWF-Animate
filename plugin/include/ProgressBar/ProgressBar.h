#pragma once

#include <thread>
#include <wx/wx.h>

#include "ProgressBar/ProgressBar.h"

#include "Components/StatusComponent.h"
#include "Components/ErrorDialog.h"

namespace sc {
	namespace Adobe {
		enum class PublisherStage : uint8_t
		{
			Initializing = 0,
			LibraryProcessing = 25,
			SpritePackaging = 50,
			Saving = 80,
			Done = 100
		};

		class ProgressBarWindow : public wxFrame
		{
		private:
			StatusComponent* CreateStatusBar(
				const wxString& title = "",
				const wxString& defaultLabel = "",
				int range = 100
			);

		public:
			ProgressBarWindow(wxString title);

		public:
			void SetLabel(const wxString& str);
			void SetLabel(const std::u16string& str);

			void SetStatus(const wxString& str);
			void SetStatus(const std::u16string& str);

			void SetProgress(uint8_t value);

			void SetAdditionalStatusBarCount(uint8_t count);
			StatusComponent* GetAvailableProgressBar();
			void DestroyProgressBar(StatusComponent* ptr);

			void ThrowException(const wxString& what);

			void ScaleByContent();

		public:
			bool aboutToExit = false;
			bool readyToExit = false;

		private:
			void OnClose(wxCloseEvent& event);

			StatusComponent* m_defProgressBar = nullptr;
			std::vector<std::pair<bool, StatusComponent*>> m_progressBars;

			wxBoxSizer* panelSizer;
			wxBoxSizer* contentSizer;
			wxPanel* panel;

			wxDECLARE_EVENT_TABLE();
		};
	}
}