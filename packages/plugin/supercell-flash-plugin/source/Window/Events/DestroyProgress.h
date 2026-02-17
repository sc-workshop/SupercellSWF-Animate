#pragma once

#include "Window/Components/StatusComponent.h"

#include <wx/wx.h>

namespace sc::Adobe {
    class PluginDestroyProgressEvent;
    wxDECLARE_EVENT(AN_EVT_DESTROY_PROGRESS, PluginDestroyProgressEvent);

    class PluginDestroyProgressEvent : public wxCommandEvent {
    public:
        PluginDestroyProgressEvent(StatusComponent* component, bool* destroyed);
        PluginDestroyProgressEvent(const PluginDestroyProgressEvent& event);

    public:
        wxEvent* Clone() const { return new PluginDestroyProgressEvent(*this); }

    public:
        StatusComponent* component;
        bool* destroyed;
    };

    typedef void (wxEvtHandler::*PluginDestroyProgressFunction)(PluginDestroyProgressEvent&);

#define SC_PluginDestroyProgressEventHandler(func) wxEVENT_HANDLER_CAST(PluginDestroyProgressFunction, func)

#define SC_EVT_DESTROY_PROGRESS(id, func)                                                                              \
    wx__DECLARE_EVT1(AN_EVT_DESTROY_PROGRESS, id, SC_PluginDestroyProgressEventHandler(func))
}