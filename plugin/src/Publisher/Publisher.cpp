#include "Publisher/Publisher.h"
#include <chrono>

namespace sc {
	namespace Adobe {
		Result Publisher::Publish(
			DOM::PIFLADocument document,
			const PIFCMDictionary publishSettings,
			const PIFCMDictionary config)
		{
			console.Init("Publisher", GetCallback());

			/*StringRep16 documentPath;
			document->GetPath(&documentPath);

			if (documentPath == NULL) {
				console.log("Document not saved. Please save document before exporting.");
				return FCM_EXPORT_FAILED;
			}*/
			
			m_config = PublisherConfig::FromDict(publishSettings);

			if (m_config.output.empty()) {
				console.log("Error. Failed to get output path");
				return FCM_EXPORT_FAILED;
			}

			FCM::Double fps;
			FCM::U_Int8 framesPerSec;

			document->GetFrameRate(fps);
			framesPerSec = (FCM::U_Int8)fps;

			FCM::FCMListPtr libraryItems;
			document->GetLibraryItems(libraryItems.m_Ptr);

			SharedWriter* writer;
			if (m_config.debug) {
				writer = new JSONWriter();
			}
			else {
				writer = new Writer();
			}

			auto start = chrono::high_resolution_clock::now();

			try {
				writer->Init(GetCallback(), m_config);
				ResourcePublisher resources(GetCallback(), writer);

				ExportLibraryItems(libraryItems, resources);

				resources.Finalize();
			}
			catch (const exception& err) {
				console.log("Error: %s", err.what());
			}

			auto end = chrono::high_resolution_clock::now();

			long long executionTime = chrono::duration_cast<chrono::seconds>(end - start).count();
			console.log("Export done by %llu second(-s)", executionTime);

			return FCM_SUCCESS;
		}

		void Publisher::ExportLibraryItems(FCMListPtr libraryItems, ResourcePublisher& resources) {
			uint32_t itemCount = 0;
			libraryItems->Count(itemCount);

			for (uint32_t i = 0; i < itemCount; i++)
			{
				AutoPtr<DOM::ILibraryItem> item = libraryItems[i];

				StringRep16 itemNamePtr;
				item->GetName(&itemNamePtr);
				u16string itemName = (const char16_t*)itemNamePtr;
				resources.GetCallocService()->Free(itemNamePtr);

				AutoPtr<DOM::LibraryItem::IFolderItem> folderItem = item;
				if (folderItem)
				{
					FCMListPtr childrens;
					folderItem->GetChildren(childrens.m_Ptr);

					// Export all its children
					ExportLibraryItems(childrens, resources);
				}
				else
				{
					AutoPtr<DOM::LibraryItem::ISymbolItem> symbolItem = item;
					if (!symbolItem) continue;

					AutoPtr<IFCMDictionary> dict;
					item->GetProperties(dict.m_Ptr);

					std::string symbolType;
					Utils::ReadString(dict, kLibProp_SymbolType_DictKey, symbolType);

					if (symbolType != "MovieClip") continue;

					uint16_t symbolIdentifer = resources.GetIdentifer(itemName);

					if (symbolIdentifer != UINT16_MAX) continue;

					resources.AddSymbol(itemName, symbolItem, true);
				}
			}
		};

		FCM::Result RegisterPublisher(PIFCMDictionary plugins, FCM::FCMCLSID docId)
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
				AutoPtr<IFCMDictionary> plugin;
				res = plugins->AddLevel(
					(const FCM::StringRep8)Utils::ToString(CLSID_Publisher).c_str(),
					plugin.m_Ptr
				);

				// Level 2 Dictionary
				AutoPtr<IFCMDictionary> category;
				res = plugin->AddLevel(
					(const FCM::StringRep8)kApplicationCategoryKey_Publisher,
					category.m_Ptr
				);

				// Level 3 Dictionary
				std::string name = PUBLISHER_NAME;
				res = category->Add(
					(const FCM::StringRep8)kApplicationCategoryKey_Name,
					kFCMDictType_StringRep8,
					(FCM::PVoid)name.c_str(),
					(FCM::U_Int32)name.length() + 1);

				std::string identifer = PUBLISHER_UNIVERSAL_NAME;
				res = category->Add(
					(const FCM::StringRep8)kApplicationCategoryKey_UniversalName,
					kFCMDictType_StringRep8,
					(FCM::PVoid)identifer.c_str(),
					(FCM::U_Int32)identifer.length() + 1);

				std::string ui = PUBLISH_SETTINGS_UI_ID;
				res = category->Add(
					(const FCM::StringRep8)kApplicationPublisherKey_UI,
					kFCMDictType_StringRep8,
					(FCM::PVoid)ui.c_str(),
					(FCM::U_Int32)ui.length() + 1);

				AutoPtr<IFCMDictionary> pDocs;
				res = category->AddLevel((const FCM::StringRep8)kApplicationPublisherKey_TargetDocs, pDocs.m_Ptr);

				// Level 4 Dictionary
				std::string empytString = ""; // TODO: ???
				res = pDocs->Add(
					(const FCM::StringRep8)Utils::ToString(docId).c_str(),
					kFCMDictType_StringRep8,
					(FCM::PVoid)empytString.c_str(),
					(FCM::U_Int32)empytString.length() + 1
				);
			}

			return res;
		}
	}
}