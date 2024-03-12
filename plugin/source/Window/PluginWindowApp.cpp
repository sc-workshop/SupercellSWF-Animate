#include "Window/PluginWindowApp.h"
#include "Module/PluginContext.h"

#include "ResourcePublisher/ResourcePublisher.h"

namespace sc {
	namespace Adobe {
		PluginWindowApp::PluginWindowApp()
		{};

		bool PluginWindowApp::OnInit() {
			PluginContext& context = PluginContext::Instance();
			PluginSessionConfig& config = PluginSessionConfig::Instance();

			FCM::StringRep16 documentPathPtr;
			config.document->GetPath(&documentPathPtr);

			fs::path documentName = fs::path((const char16_t*)documentPathPtr).filename();
			context.falloc->Free(documentPathPtr);

			window = new PluginWindow(
				context.locale.GetString("TID_WINDOW_TITLE", documentName.u16string().c_str())
			);

			{
				wxIcon icon;
				fs::path iconPath = PluginContext::CurrentPath(PluginContext::PathType::Assets) / "window.ico";
				icon.LoadFile(iconPath.wstring(), wxBITMAP_TYPE_ICO);
				if (icon.IsOk()) {
					window->SetIcon(icon);
				}
			}

			{
				fs::path animFolder = PluginContext::CurrentPath(PluginContext::PathType::Assets) / "loading";
				std::vector<std::wstring> files;

				for (const auto& entry : fs::directory_iterator(animFolder)) {
					files.push_back(entry.path().wstring());
				}

				wxAnimationCtrl* animation = new wxAnimationCtrl(window, wxID_ANY, wxNullAnimation, wxPoint(335, 5));
				animation->LoadFile(files[rand() % files.size()]);
				animation->Play();
			}

			window->Show();
			SetTopWindow(window);

			isInited = true;
			return true;
		}
	}
}