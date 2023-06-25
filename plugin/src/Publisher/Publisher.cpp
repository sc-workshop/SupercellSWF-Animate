#include "Publisher/Publisher.h"
#include "Module/AppContext.h"
#include <chrono>

#include "Ui/PluginUI.h"

namespace sc {
	namespace Adobe {
		FCM::Result Publisher::Publish(
			DOM::PIFLADocument document,
			const FCM::PIFCMDictionary publishSettings,
			const FCM::PIFCMDictionary config
		){
			AppContext app(GetCallback(), document, publishSettings);

			auto start = chrono::high_resolution_clock::now();

			app.window = new PluginUI(app);
			wxApp::SetInstance(app.window);
			wxEntry();

			auto end = chrono::high_resolution_clock::now();

			long long executionTime = chrono::duration_cast<chrono::seconds>(end - start).count();
			app.trace("Done by %llu second(-s)", executionTime);

			return FCM_SUCCESS;
		}

		FCM::Result RegisterPublisher(FCM::PIFCMDictionary plugins, FCM::FCMCLSID docId)
		{
			FCM::Result res;

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
				res = plugins->AddLevel(
					(const FCM::StringRep8)Utils::ToString(CLSID_Publisher).c_str(),
					plugin.m_Ptr
				);

				// Level 2 Dictionary
				FCM::AutoPtr<FCM::IFCMDictionary> category;
				res = plugin->AddLevel(
					(const FCM::StringRep8)kApplicationCategoryKey_Publisher,
					category.m_Ptr
				);

				// Level 3 Dictionary
				std::string name = PUBLISHER_NAME;
				res = category->Add(
					(const FCM::StringRep8)kApplicationCategoryKey_Name,
					FCM::kFCMDictType_StringRep8,
					(FCM::PVoid)name.c_str(),
					(FCM::U_Int32)name.length() + 1);

				std::string identifer = PUBLISHER_UNIVERSAL_NAME;
				res = category->Add(
					(const FCM::StringRep8)kApplicationCategoryKey_UniversalName,
					FCM::kFCMDictType_StringRep8,
					(FCM::PVoid)identifer.c_str(),
					(FCM::U_Int32)identifer.length() + 1);

				std::string ui = PUBLISH_SETTINGS_UI_ID;
				res = category->Add(
					(const FCM::StringRep8)kApplicationPublisherKey_UI,
					FCM::kFCMDictType_StringRep8,
					(FCM::PVoid)ui.c_str(),
					(FCM::U_Int32)ui.length() + 1);

				FCM::AutoPtr<FCM::IFCMDictionary> pDocs;
				res = category->AddLevel((const FCM::StringRep8)kApplicationPublisherKey_TargetDocs, pDocs.m_Ptr);

				// Level 4 Dictionary
				std::string empytString = ""; // TODO: ???
				res = pDocs->Add(
					(const FCM::StringRep8)Utils::ToString(docId).c_str(),
					FCM::kFCMDictType_StringRep8,
					(FCM::PVoid)empytString.c_str(),
					(FCM::U_Int32)empytString.length() + 1
				);
			}

			return res;
		}
	}
}