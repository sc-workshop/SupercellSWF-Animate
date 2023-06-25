#include "Ui/PluginUI.h"

#include "Module/AppContext.h"

#include "Publisher/ResourcePublisher.h"

namespace sc {
	namespace Adobe {
		PluginUI::PluginUI(AppContext& context) : m_context(context)
		{

		};

		bool PluginUI::OnInit() {
			ui = new PluginUIWindow(wxStringU16(m_context.locale.Get("TID_WINDOW_TITLE")));
			ui->SetAdditionalStatusBarCount(2);

			{
				wxIcon icon;
				fs::path iconPath = Utils::CurrentPath() / "../res/assets/scwmake.ico";
				icon.LoadFile(iconPath.wstring(), wxBITMAP_TYPE_ICO);
				if (icon.IsOk()) {
					ui->SetIcon(icon);
				}
			}

			{
				fs::path animFolder = Utils::CurrentPath() / "../res/assets/loading/";
				std::vector<std::wstring> files;

				for (const auto& entry : fs::directory_iterator(animFolder)) {
					files.push_back(entry.path().wstring());
				}

				int fileIndex = 1 + (rand() % files.size());

				wxAnimationCtrl* animationCtrl = new wxAnimationCtrl(ui, wxID_ANY, wxNullAnimation, wxPoint(340, 5));
				animationCtrl->LoadFile(files[fileIndex - 1]);
				animationCtrl->Play();
			}

			ui->SetLabel(wxStringU16(m_context.locale.Get("TID_BAR_LABEL_TOTAL")));
			ui->SetStatus(wxStringU16(m_context.locale.Get("TID_STATUS_INIT")));

			ui->Show(true);

			switch (m_context.config.mode)
			{
			case sc::Adobe::PublisherMode::Export:
			{
				std::thread publisher([this]() {
					try {
						ResourcePublisher::Publish(this->m_context);
					}
					catch (const std::exception& exception) {
						this->m_context.window->ui->ThrowException(exception.what());
					}
				});
				publisher.detach();
			}
				break;
			case sc::Adobe::PublisherMode::Import:
			default:
				break;
			}

			return true;
		}

	}
}