#pragma once

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

#include <wx/wx.h>
#include <wx/animate.h>
#include <functional>
#include <mutex>

#include "PluginWindow.h"

namespace sc {
	namespace Adobe {
		class PluginWindowApp : public wxApp
		{
		public:
			PluginWindowApp();
			virtual ~PluginWindowApp() = default;
            
        public:
            // Wait until app will be ready
            void Wait();

		public:
			PluginWindow* window = nullptr;

			virtual bool OnInit();
            
        private:
            std::mutex m_mutex;
            std::condition_variable m_cv;
            bool m_ready;
		};
	}
}
