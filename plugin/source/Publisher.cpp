#include "Publisher.h"
#include <shared_mutex>

namespace sc {
	namespace Adobe {
		FCM::Result SCPublisher::Publish(
			DOM::PIFLADocument document,
			const FCM::PIFCMDictionary publishSettings,
			const FCM::PIFCMDictionary appConfig
		) {
			PluginContext& context = PluginContext::Instance();

			context.logger->info("--------------------------- Called Publish -----------------------------");

			PluginSessionConfig::Clear();
			PluginSessionConfig& config = PluginSessionConfig::Instance();
			config.document = document;
			config.FromDict(publishSettings);
			config.Normalize();

			auto start = std::chrono::high_resolution_clock::now();

			std::shared_mutex publishing_ui;
			// Must be unlocked when ui is ready to use
			publishing_ui.lock();

			context.logger->info("Starting UI thread...");
			std::thread progressWindow(
				[&context, &publishing_ui]()
				{
					context.logger->info("Windows intializing has started...");
					context.InitializeWindow();

					bool entry_start_status = wxEntryStart(0, nullptr);
					context.logger->info("Windows entry initializing finished with status: {}", entry_start_status);
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

					context.logger->info("Unlocking worker mutex...");
					publishing_ui.unlock();

					wxTheApp->OnRun();
					wxTheApp->OnExit();
					wxEntryCleanup();

					context.logger->info("Destroying window...");
					context.DestroyWindow();
				}
			);

			context.logger->info("Starting Publisher thread...");
			FCM::Result result = FCM_SUCCESS;
			std::thread publishing([&context, &result, &publishing_ui]()
				{
					// Block thread until publishing ui is ready
					publishing_ui.lock();
					context.logger->info("Publishing thread has started...");

					// Removes Exception catch in debug mode
#if !(SC_DEBUG)
					try {
#endif
						SCWriter writer;
						ResourcePublisher publisher(writer);
						publisher.Publish();

						context.Window()->readyToExit = true;
						context.Window()->Close();
#if !(SC_DEBUG)
					}
					catch (const PluginException& exception)
					{
						context.Window()->ThrowException((wchar_t*)exception.Title());
						context.Trace(u"%s\n%s", exception.Title(), exception.Description());
						context.Window()->readyToExit = true;
						result = FCM_EXPORT_FAILED;
					}
					catch (const sc::GeneralRuntimeException& exception) {
						const size_t exception_buffer_size = 1024;
						char description_buffer[exception_buffer_size] = { 0 };
						std::snprintf(description_buffer, exception_buffer_size, "[%s] %s", exception.type(), exception.what());

						context.Window()->ThrowException(exception.what());
						context.Trace(exception.message());
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
#endif
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

			return result;
	}

		FCM::Result RegisterPublisher(FCM::PIFCMDictionary plugins, FCM::FCMCLSID docId)
		{
			FCM::Result result;

			/*
			 * Dictionary structure for a Publisher plugin is as follows:
			 *
			 *  Level 0 :
			 *              --------------------------------
			 *             | Application.Component |  ----- | -----------------------------
			 *              --------------------------------                               |
			 *                                                                             |
			 *  Level 1:                                   <-------------------------------
			 *              ------------------------------
			 *             | CLSID_Publisher_GUID | ----- | -------------------------------
			 *              ------------------------------                                 |
			 *                                                                             |
			 *  Level 2:                                      <----------------------------
			 *              ---------------------------------------------------
			 *             | Application.Component.Category.Publisher |  ----- |-----------
			 *              ---------------------------------------------------            |
			 *                                                                             |
			 *  Level 3:                                                           <-------
			 *              -------------------------------------------------------------------------
			 *             | Application.Component.Category.Name           | PUBLISHER_NAME          |
			 *              -------------------------------------------------------------------------|
			 *             | Application.Component.Category.UniversalName  | PUBLISHER_UNIVERSAL_NAME|
			 *              -------------------------------------------------------------------------|
			 *             | Application.Component.Publisher.UI            | PUBLISH_SETTINGS_UI_ID  |
			 *              -------------------------------------------------------------------------|
			 *             | Application.Component.Publisher.TargetDocs    |              -----------|--
			 *              -------------------------------------------------------------------------| |
			 *                                                                                         |
			 *  Level 4:                                                    <--------------------------
			 *              -----------------------------------------------
			 *             | CLSID_DocType   |  Empty String               |
			 *              -----------------------------------------------
			 *
			 *  Note that before calling this function the level 0 dictionary has already
			 *  been added. Here, the 1st, 2nd and 3rd level dictionaries are being added.
			 */

			{
				// Level 1 Dictionary
				FCM::AutoPtr<FCM::IFCMDictionary> plugin;
				result = plugins->AddLevel(
					((std::string)CLSID_Publisher).c_str(),
					plugin.m_Ptr
				);

				// Level 2 Dictionary
				FCM::AutoPtr<FCM::IFCMDictionary> category;
				result = plugin->AddLevel(
					(const FCM::StringRep8)kApplicationCategoryKey_Publisher,
					category.m_Ptr
				);

				// Level 3 Dictionary
				std::string name = PUBLISHER_NAME;
				result = category->Add(
					(const FCM::StringRep8)kApplicationCategoryKey_Name,
					FCM::kFCMDictType_StringRep8,
					(FCM::PVoid)name.c_str(),
					(FCM::U_Int32)name.length() + 1);

				std::string identifer = PUBLISHER_UNIVERSAL_NAME;
				result = category->Add(
					(const FCM::StringRep8)kApplicationCategoryKey_UniversalName,
					FCM::kFCMDictType_StringRep8,
					(FCM::PVoid)identifer.c_str(),
					(FCM::U_Int32)identifer.length() + 1);

				std::string ui = PUBLISH_SETTINGS_UI_ID;
				result = category->Add(
					(const FCM::StringRep8)kApplicationPublisherKey_UI,
					FCM::kFCMDictType_StringRep8,
					(FCM::PVoid)ui.c_str(),
					(FCM::U_Int32)ui.length() + 1);

				FCM::AutoPtr<FCM::IFCMDictionary> pDocs;
				result = category->AddLevel((const FCM::StringRep8)kApplicationPublisherKey_TargetDocs, pDocs.m_Ptr);

				// Level 4 Dictionary
				std::string empytString = ""; // TODO: ???
				result = pDocs->Add(
					((std::string)docId).c_str(),
					FCM::kFCMDictType_StringRep8,
					(FCM::PVoid)empytString.c_str(),
					(FCM::U_Int32)empytString.length() + 1
				);
			}

			return result;
		}
}
}