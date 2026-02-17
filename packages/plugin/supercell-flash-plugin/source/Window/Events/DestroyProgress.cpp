#include "Window/Events/DestroyProgress.h"

namespace sc::Adobe {
    wxDEFINE_EVENT(AN_EVT_DESTROY_PROGRESS, PluginDestroyProgressEvent);

    PluginDestroyProgressEvent::PluginDestroyProgressEvent(StatusComponent* component, bool* destroyed) :
        wxCommandEvent(AN_EVT_DESTROY_PROGRESS, wxID_ANY),
        component(component),
        destroyed(destroyed) {};

    PluginDestroyProgressEvent::PluginDestroyProgressEvent(const PluginDestroyProgressEvent& event) {
        component = event.component;
        destroyed = event.destroyed;
    };
}