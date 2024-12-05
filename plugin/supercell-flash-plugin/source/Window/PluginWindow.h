#pragma once

#include <mutex>
#include <thread>
#include <wx/wx.h>

#include "Components/StatusComponent.h"
#include "Components/ErrorDialog.h"
#include "Events/CreateProgress.h"
#include "Events/DestroyProgress.h"

namespace sc {
	namespace Adobe {
		class PluginWindow : public wxFrame
		{
		public:
			StatusComponent* CreateStatusBarComponent(
				const std::u16string& title = u"",
				const std::u16string& defaultLabel = u"",
				int range = 100
			);

			void DestroyStatusBar(StatusComponent* bar);

		public:
			PluginWindow(wxString& title);

		private:
			PluginWindow(PluginWindow&) = delete;

		public:
			void ThrowException(const wxString& what);

		public:
			bool aboutToExit = false;
			bool readyToExit = false;

		private:
			void OnClose(wxCloseEvent& event);
			void OnProgressCreate(PluginCreateProgressEvent& event);
			void OnProgressDestroy(PluginDestroyProgressEvent& event);

			void ScaleByContent();

			wxBoxSizer* panelSizer;
			wxBoxSizer* contentSizer;
			wxPanel* panel;

			std::condition_variable m_window_cv;
			std::mutex m_window_mut;

			uint32_t m_elements_counter = 0;

			wxDECLARE_EVENT_TABLE();
		};
	}
}