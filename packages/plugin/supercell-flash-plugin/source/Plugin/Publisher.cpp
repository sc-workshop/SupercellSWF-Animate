#include "Publisher.h"

#include <shared_mutex>

#include "Module/Module.h"
#include "Writer/Writer.h"
#include "Module/SCPluginException.h"

using namespace Animate::Publisher;

namespace sc {
	namespace Adobe {
		void SCPublisher::Publish(const SCConfig& /*config*/)
		{
			SCPlugin& context = SCPlugin::Instance();

			context.logger->info("--------------------------- Called Publish -----------------------------");

			auto start = std::chrono::high_resolution_clock::now();

			std::shared_mutex publishing_ui;
			// Must be unlocked when ui is ready to use
			publishing_ui.lock();

			context.logger->info("Starting UI...");
			std::thread progressWindow(
				[&context, &publishing_ui]()
				{;
					context.InitializeWindow();
					bool entry_start_status = wxEntryStart(0, nullptr);
					
					if (!entry_start_status)
					{
						auto message = wxSysErrorMsgStr(0);
						context.logger->error("wxSysErrorMsgStr: {}", message.ToStdString());
					}

					bool init_status = wxTheApp->CallOnInit();
					context.logger->info("Windows init finished with status: {}", init_status);
					if (!init_status)
					{
						auto message = wxSysErrorMsgStr(0);
						context.logger->error("wxSysErrorMsgStr: {}", message.ToStdString());
					}

					context.logger->info("UI successfully started");
					publishing_ui.unlock();

					wxTheApp->OnRun();
					wxEntryCleanup();
				}
			);

			FCM::Result result = FCM_SUCCESS;
			std::thread publishing([&context, &result, &publishing_ui, this]()
				{
					// Block thread until publishing ui is ready
					publishing_ui.lock();
					context.logger->info("Starting publishing...");

					try {
						DoPublish();
					}
					catch (const FCM::FCMPluginException& exception)
					{
						std::string reason;
						{
							std::stringstream message;
							auto& symbol = exception.Symbol();
							if (!symbol.name.empty())
							{
								message << " [" << FCM::Locale::ToUtf8(symbol.name) << "] ";
							}
							message << exception.what();
							reason = message.str();
						}

						context.Window()->ThrowException(reason);
						context.Window()->readyToExit = true;
						result = FCM_EXPORT_FAILED;
					}
					catch (const std::exception& exception) {
						context.Window()->ThrowException(exception.what());
						context.Window()->readyToExit = true;
						result = FCM_EXPORT_FAILED;
					}
					catch (...) {
						context.logger->error("Publishing finished with unknown exception!");

						context.Window()->readyToExit = true;
						result = FCM_EXPORT_FAILED;
					}
					publishing_ui.unlock();
				}
			);

			publishing.join();
			progressWindow.join();

			context.logger->info("Publisher finished with status: {}", (uint32_t)result);

			auto end = std::chrono::high_resolution_clock::now();

			long long int executionTime = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
			context.logger->info("Execution time: {}s", executionTime);
			context.logger->info("------------------------------------------------------------------------");

			context.Trace(
				context.locale.GetString("TID_EXPORT_TIME_STATUS", executionTime)
			);
		}

		void SCPublisher::DoPublish()
		{
			SCPlugin& context = SCPlugin::Instance();
			const SCConfig& config = SCPlugin::Publisher::ActiveConfig();

			StatusComponent* publishStatus = context.Window()->CreateStatusBarComponent(
				context.locale.GetString("TID_STATUS_INIT")
			);

			SCWriter writer;
			ResourcePublisher publisher(writer);

			{
				if (config.exportToExternal)
				{
					if (fs::exists(config.exportToExternalPath))
					{
						publishStatus->SetStatusLabel(context.locale.GetString("TID_EXTERNAL_FILE_LOAD"));
						uint16_t offset = writer.LoadExternal(config.exportToExternalPath);
						publisher.SetIdOffset(offset);
					}
					else
					{
						throw SCPluginException("TID_SWF_MISSING_EXTERNAL_FILE", config.exportToExternalPath.wstring().c_str());
					}
				}
			}

			{
				fs::path document_path = context.falloc->GetString16(
					config.activeDocument,
					&Animate::DOM::IFLADocument::GetPath
				);

				publishStatus->SetStatusLabel(
					context.locale.GetString(
						"TID_WINDOW_TITLE", 
						document_path.filename().u16string().c_str()
					)
				);

				publisher.PublishDocument(config.activeDocument);
			}
			

			publishStatus->SetStatus(
				context.locale.GetString("TID_STATUS_SAVE")
			);

			publisher.Finalize();
			context.DestroyWindow();
		}
	}
}