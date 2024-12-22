#pragma once

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include <wx/wx.h>
#include <wx/animate.h>
#include <functional>

#include "PluginWindow.h"

namespace sc {
	namespace Adobe {
		class PluginWindowApp : public wxApp
		{
		public:
			PluginWindowApp();
			virtual ~PluginWindowApp() = default;

		public:
			PluginWindow* window = nullptr;

			virtual bool OnInit();
		};
	}
}