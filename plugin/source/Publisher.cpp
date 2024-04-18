#include "Publisher.h"

#if SC_MSVC
#define TIME_TYPE "%I64d"
#else
#define TIME_TYPE "%lld"
#endif

namespace sc {
	namespace Adobe {
		FCM::Result SCPublisher::Publish(
			DOM::PIFLADocument document,
			const FCM::PIFCMDictionary publishSettings,
			const FCM::PIFCMDictionary appConfig
		) {
			PluginContext& context = PluginContext::Instance();

			PluginSessionConfig::Clear();
			PluginSessionConfig& config = PluginSessionConfig::Instance();
			config.document = document;
			config.FromDict(publishSettings);
			config.Normalize();

			auto start = std::chrono::high_resolution_clock::now();

			bool isWindowReady = false;
			std::thread progressWindow(
				[&context, &isWindowReady]()
				{
					context.InitializeWindow();
					wxEntryStart(0, nullptr);
					wxTheApp->CallOnInit();

					isWindowReady = true;

					wxTheApp->OnRun();
					wxTheApp->OnExit();
					wxEntryCleanup();

					context.DestroyWindow();
				}
			);

			FCM::Result result = FCM_SUCCESS;
			std::thread publishing([&context, &result, &isWindowReady]()
				{
					// Do nothing until window is ready
					while (!isWindowReady);

					// Removes Exception catch in debug mode
#if !(SC_DEBUG)
					try {
#endif
						SCWriter writer;
						ResourcePublisher publisher(writer);
						publisher.Publish();

#if !(SC_DEBUG)
					}
					catch (const PluginException& exception)
					{
						context.window()->ThrowException((wchar_t*)exception.Title());
						context.Trace(u"%s\n%s", exception.title(), exception.Description());
						result = FCM_EXPORT_FAILED;
					}
					catch (const sc::GeneralRuntimeException& exception) {
						context.window()->ThrowException(exception.what());
						context.Trace(exception.message());
						result = FCM_EXPORT_FAILED;
					}
					catch (...) {
						context.Trace(
							context.locale.GetString("TID_UNKNOWN_EXCEPTION")
						);
						result = FCM_EXPORT_FAILED;
					}
#endif

					context.Window()->readyToExit = true;
					context.Window()->Close();
				}
			);

			publishing.join();
			progressWindow.join();

			auto end = std::chrono::high_resolution_clock::now();

			long long int executionTime = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

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