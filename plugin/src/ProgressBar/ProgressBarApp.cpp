#include "ProgressBar/ProgressBarApp.h"
#include "Module/Context.h"

#include "ResourcePublisher/ResourcePublisher.h"

namespace sc {
	namespace Adobe {
		ProgressBarApp::ProgressBarApp(Context& context, std::function<void()> initCallback) : m_context(context), m_init(initCallback)
		{
		};

		bool ProgressBarApp::OnInit() {
			window = new ProgressBarWindow(wxStringU16(m_context.locale.Get("TID_WINDOW_TITLE")));
			window->SetAdditionalStatusBarCount(2);

			{
				wxIcon icon;
				fs::path iconPath = Utils::CurrentPath() / "../res/assets/scwmake.ico";
				icon.LoadFile(iconPath.wstring(), wxBITMAP_TYPE_ICO);
				if (icon.IsOk()) {
					window->SetIcon(icon);
				}
			}

			{
				fs::path animFolder = Utils::CurrentPath() / "../res/assets/loading/";
				std::vector<std::wstring> files;

				for (const auto& entry : fs::directory_iterator(animFolder)) {
					files.push_back(entry.path().wstring());
				}

				wxAnimationCtrl* animationCtrl = new wxAnimationCtrl(window, wxID_ANY, wxNullAnimation, wxPoint(340, 5));
				animationCtrl->LoadFile(files[rand() % files.size()]);
				animationCtrl->Play();
			}

			window->SetLabel(wxStringU16(m_context.locale.Get("TID_BAR_LABEL_TOTAL")));
			window->SetStatus(wxStringU16(m_context.locale.Get("TID_STATUS_INIT")));

			window->Show(true);
			SetTopWindow(window);

			m_init();

			return true;
		}
	}
}