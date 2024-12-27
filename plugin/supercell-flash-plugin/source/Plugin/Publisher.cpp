#include "Publisher.h"

#include <shared_mutex>

#include "Module/Module.h"
#include "Writer/Writer.h"

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
					wxTheApp->OnExit();
					wxEntryCleanup();

					context.logger->info("Destroying UI...");
					context.DestroyWindow();
				}
			);

			FCM::Result result = FCM_SUCCESS;
			std::thread publishing([&context, &result, &publishing_ui, this]()
				{
					// Block thread until publishing ui is ready
					publishing_ui.lock();
					context.logger->info("Starting publishing...");

					// Removes Exception catch in debug mode

//#if !(WK_DEBUG)
					try {
//#endif
						DoPublish();
//#if !(WK_DEBUG)
					}
					catch (const FCM::FCMPluginException& exception)
					{
						context.Window()->ThrowException(""); // TODO

						context.Window()->readyToExit = true;
						result = FCM_EXPORT_FAILED;
					}
					catch (const wk::Exception& exception) {
						context.Window()->ThrowException(exception.what());
						context.Trace(exception.what());
						context.Window()->readyToExit = true;
						result = FCM_EXPORT_FAILED;
					}
					catch (...) {
						context.Trace(
							context.locale.GetString("TID_UNKNOWN_EXCEPTION")
						);

						context.logger->error("Publishing finished with unknown exception");

						context.Window()->readyToExit = true;
						result = FCM_EXPORT_FAILED;
					}
//#endif
					publishing_ui.unlock();
				}
			);

			publishing.join();
			progressWindow.join();

			context.logger->info("Publisher finished with status: {}", (uint32_t)result);

			auto end = std::chrono::high_resolution_clock::now();

			long long int executionTime = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
			context.logger->info("Execution time: {}", executionTime);
			context.logger->info("------------------------------------------------------------------------");

			context.Trace(
				context.locale.GetString("TID_EXPORT_TIME_STATUS", executionTime)
			);
		}

		void SCPublisher::DoPublish()
		{
			SCPlugin& context = SCPlugin::Instance();
			const SCConfig& config = SCPlugin::Publisher::ActiveConfig();

			SCWriter writer;
			ResourcePublisher publisher(writer);

			StatusComponent* publishStatus = context.Window()->CreateStatusBarComponent(
				context.locale.GetString("TID_STATUS_INIT")
			);

			{
				FCM::StringRep16 document_path_str = nullptr;
				config.activeDocument->GetPath(&document_path_str);
				fs::path document_path(std::u16string((const char16_t*)document_path_str));

				publishStatus->SetStatusLabel(
					context.locale.GetString(
						"TID_WINDOW_TITLE", 
						document_path.filename().u16string().c_str()
					)
				);

				context.falloc->Free(document_path_str);

				publisher.PublishDocument(config.activeDocument);
			}
			

			publishStatus->SetStatus(
				context.locale.GetString("TID_STATUS_SAVE")
			);

			publisher.Finalize();

			context.Window()->readyToExit = true;
			context.Window()->Close();
		}
	}
}