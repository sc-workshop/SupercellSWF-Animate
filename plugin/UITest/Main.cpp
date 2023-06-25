#pragma once

#include <any>
#include <wx/wx.h>
#include <thread>
#include <chrono>
#include <iostream>

#include "Ui/Window.h"
#include "Ui/Components/ErrorDialog.h"

using namespace std;
using namespace chrono;

#ifdef __WXMSW__
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <wx/msw/msvcrt.h>
#endif

namespace sc {
	namespace Adobe {
		void UiTestFunction(sc::Adobe::PluginUIWindow* ui) {
			this_thread::sleep_for(seconds(3));

			ui->SetProgress((uint8_t)PublisherExportStage::LibraryProcessing);
			ui->SetStatus("Library processing...");

			ProgressBar* itemsProgress = ui->GetAvailableProgressBar();
			itemsProgress->SetLabel("Items :");
			itemsProgress->SetRange(5);
			for (int i = 1; 5 >= i; i++) {
				if (ui->aboutToExit) {
					ui->readyToExit = true;
					ui->Close(true);
					exit(0);
				}

				itemsProgress->SetProgress(i);
				itemsProgress->SetStatus("Item: " + std::to_string(i));

				this_thread::sleep_for(milliseconds(700));
			}

			ui->DestroyProgressBar(itemsProgress);

			this_thread::sleep_for(seconds(1));

			ui->SetProgress((uint8_t)PublisherExportStage::SpritePackaging);
			ui->SetStatus("Texture creating...");

			ProgressBar* spritesProgress = ui->GetAvailableProgressBar();
			spritesProgress->SetLabel("Sprites :");
			spritesProgress->SetRange(15);
			for (int i = 1; 15 >= i; i++) {
				if (ui->aboutToExit) {
					ui->readyToExit = true;
					ui->Close(true);
					exit(0);
				}
				spritesProgress->SetProgress(i);
				spritesProgress->SetStatus("Sprite: " + std::to_string(i));

				this_thread::sleep_for(milliseconds(400));
			}

			ui->DestroyProgressBar(spritesProgress);

			this_thread::sleep_for(seconds(1));

			ui->SetProgress((uint8_t)PublisherExportStage::Saving);
			ui->SetStatus("Saving...");

			this_thread::sleep_for(seconds(1));

			ui->SetProgress((uint8_t)PublisherExportStage::Done);

			throw std::exception("Test exception");
		}
	}
}

namespace sc {
	namespace Adobe {
		class UI : public wxApp
		{
		public:
			PluginUIWindow* ui = nullptr;

			virtual bool OnInit() 
			{
				ui = new PluginUIWindow("Test");
				ui->Show(true);
				ui->SetAdditionalStatusBarCount(2);

				std::thread worker([this]() {
					try {
						UiTestFunction(this->ui);
					}
					catch (const std::exception& exception) {
						this->ui->ThrowException(exception.what());
					}
					});
				worker.detach();

				return true;
			}
		};
	}
}

int main() {
	new sc::Adobe::UI();
	return wxEntry();
}