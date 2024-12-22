#include "Module/Module.h"
#include "Window/PluginWindowApp.h"

#include <random>

namespace sc {
	namespace Adobe {
		PluginWindowApp::PluginWindowApp()
		{};

		bool PluginWindowApp::OnInit() {
			SCPlugin& context = SCPlugin::Instance();
			
			FCM::StringRep16 documentPathPtr;
			
			const SCConfig& config = SCPlugin::Publisher::ActiveConfig();
			config.activeDocument->GetPath(&documentPathPtr);
			
			fs::path documentName = "Untitled";
			if (documentPathPtr)
			{
				documentName = fs::path((const char16_t*)documentPathPtr).filename();
				context.falloc->Free(documentPathPtr);
			}
			
			context.logger->info("Document name: {}", documentName.string());
			
			window = new PluginWindow(wxString(DOCTYPE_NAME));
			
			{
				wxIcon icon;
				fs::path iconPath = SCPlugin::CurrentPath(SCPlugin::PathType::Assets) / "window.ico";
				context.logger->info("Loading icon by path: {}", iconPath.string());
				icon.LoadFile(iconPath.wstring(), wxBITMAP_TYPE_ICO);
				if (icon.IsOk()) {
					window->SetIcon(icon);
				}
			}
			
			{
				fs::path animFolder = SCPlugin::CurrentPath(SCPlugin::PathType::Assets) / "loading";
				context.logger->info("Anims storage path: {}", animFolder.string());
			
				std::vector<std::wstring> files;
			
				for (const auto& entry : fs::directory_iterator(animFolder)) {
					files.push_back(entry.path().wstring());
				}
			
				size_t animation_index = 0;
			
				{
					std::random_device seed;
					std::mt19937 generator(seed());
					std::uniform_int_distribution<> distr(0, files.size() - 1);
			
					animation_index = distr(generator);
				}
			
				context.logger->info("Animation index: {}", animation_index);
				wxAnimationCtrl* animation = new wxAnimationCtrl(window, wxID_ANY, wxNullAnimation, wxPoint(335, 5));
				if (animation->LoadFile(files[animation_index]))
				{
					animation->Play();
				};
			}
			
			context.logger->info("Calling window->Show()");
			bool window_showed = window->Show(true);
			if (!window_showed)
			{
				auto message = wxSysErrorMsgStr(0);
				context.logger->error("wxSysErrorMsgStr: {}", message.ToStdString());
			}
			
			context.logger->info("Window succesfully inited");
			return true;
		}
	}
}