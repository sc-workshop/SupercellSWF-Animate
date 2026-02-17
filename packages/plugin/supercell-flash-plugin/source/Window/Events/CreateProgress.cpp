#include "Window/Events/CreateProgress.h"

namespace sc::Adobe {
    wxDEFINE_EVENT(AN_EVT_CREATE_PROGRESS, PluginCreateProgressEvent);

    PluginCreateProgressEvent::PluginCreateProgressEvent(const std::u16string& title,
                                                         const std::u16string& status,
                                                         const int& range,
                                                         StatusComponent*& component) :
        wxCommandEvent(AN_EVT_CREATE_PROGRESS, wxID_ANY),
        title(title),
        status(status),
        range(range),
        result(component) {
    }

    PluginCreateProgressEvent::PluginCreateProgressEvent(const PluginCreateProgressEvent& event) :
        wxCommandEvent(event),
        title(event.title),
        status(event.status),
        range(event.range),
        result(event.result) {
    }
}