#pragma once

#include <wx/wx.h>
#include <wx/animate.h>
#include <functional>

#include "ProgressBar.h"

namespace sc {
	namespace Adobe {
		class Context;

		class ProgressBarApp : public wxApp
		{
			Context& m_context;
			std::function<void()> m_init;

		public:
			ProgressBarApp(Context& context, std::function<void()> initCallback);

		public:
			ProgressBarWindow* window = nullptr;

			virtual bool OnInit();
		};
	}
}