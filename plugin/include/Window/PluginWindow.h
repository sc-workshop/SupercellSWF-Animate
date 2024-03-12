#pragma once

#include <thread>
#include <wx/wx.h>

#include "Components/StatusComponent.h"
#include "Components/ErrorDialog.h"
#include "Events/CreateProgress.h"

namespace sc {
	namespace Adobe {
		class PluginWindow : public wxFrame
		{
		public:
			StatusComponent* CreateStatusBar(
				const std::u16string& title = u"",
				const std::u16string& defaultLabel = u"",
				int range = 100
			);

		public:
			PluginWindow(const std::u16string& title);

		public:
			void ThrowException(const wxString& what);

		public:
			bool aboutToExit = false;
			bool readyToExit = false;

		private:
			void OnClose(wxCloseEvent& event);
			void OnProgressCreate(PluginCreateProgressEvent& event);

			void ScaleByContent();

			wxBoxSizer* panelSizer;
			wxBoxSizer* contentSizer;
			wxPanel* panel;

			wxDECLARE_EVENT_TABLE();
		};
	}
}