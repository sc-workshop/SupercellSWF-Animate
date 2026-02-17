#pragma once

#include "Components/ErrorDialog.h"
#include "Components/StatusComponent.h"
#include "Events/CreateProgress.h"
#include "Events/DestroyProgress.h"

#include <mutex>
#include <thread>
#include <wx/wx.h>

namespace sc::Adobe {
    class PluginWindow : public wxDialog {
    public:
        StatusComponent* CreateStatusBarComponent(const std::u16string& title = u"",
                                                  const std::u16string& defaultLabel = u"",
                                                  int range = 100);

        void DestroyStatusBar(StatusComponent* bar);

    public:
        PluginWindow(wxString title);

    private:
        PluginWindow(PluginWindow&) = delete;

    public:
        void Wait();
        void ThrowException(wxString what);

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
        bool m_ready = false;

        uint32_t m_elements_counter = 0;

        wxDECLARE_EVENT_TABLE();
    };
}
