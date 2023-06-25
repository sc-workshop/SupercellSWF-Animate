#pragma once

#include <wx/wx.h>
#include <wx/animate.h>

#include "Window.h"
#include "Module/Config.h"

namespace sc {
	namespace Adobe {
		class AppContext;

		class PluginUI : public wxApp
		{
			AppContext& m_context;

		public:
			PluginUI(AppContext& context);

		public:
			PluginUIWindow* ui = nullptr;

			virtual bool OnInit();
		};
	}
}