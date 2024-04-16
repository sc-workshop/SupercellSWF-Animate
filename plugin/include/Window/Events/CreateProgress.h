#pragma once

#include "Window/Components/StatusComponent.h"
#include <wx/wx.h>

namespace sc
{
	namespace Adobe
	{
		class PluginCreateProgressEvent;
		wxDECLARE_EVENT(AN_EVT_CREATE_PROGRESS, PluginCreateProgressEvent);

		class PluginCreateProgressEvent : public wxCommandEvent
		{
		public:
			PluginCreateProgressEvent(
				const std::u16string& title,
				const std::u16string& status,
				const int& range,
				StatusComponent*& component
			);

			PluginCreateProgressEvent(const PluginCreateProgressEvent& event);

		public:
			wxEvent* Clone() const { return new PluginCreateProgressEvent(*this); }

		public:
			const std::u16string& title;
			const std::u16string& status;
			const int& range;

			// Reference to Progress Bar Pointer
			StatusComponent*& result;
		};

		typedef void (wxEvtHandler::* PluginCreateProgressFunction)(PluginCreateProgressEvent&);

#define SC_PluginCreateProgressEventHandler(func) wxEVENT_HANDLER_CAST(PluginCreateProgressFunction, func)

#define SC_EVT_CREATE_PROGRESS(id, func) \
 	wx__DECLARE_EVT1(AN_EVT_CREATE_PROGRESS, id, SC_PluginCreateProgressEventHandler(func))
	}
}