#include "Publisher/Publisher.h"

namespace sc {
	namespace Adobe {
		Result Publisher::Publish(
			DOM::PIFLADocument document,
			const PIFCMDictionary publishSettings,
			const PIFCMDictionary config)
		{
			FCM::Result res;
			FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
			FCM::AutoPtr<FCM::IFCMCalloc> calloc;

			console.Init("Publisher", GetCallback());
			m_config = PublisherConfig::FromDict(publishSettings);

			if (m_config.output.empty()) {
				console.log("Error. Failed to get output path");
				return FCM_EXPORT_FAILED;
			}

			calloc = Utils::GetCallocService(GetCallback());
			ASSERT(calloc.m_Ptr != NULL);

			FCM::Double fps;
			FCM::U_Int8 framesPerSec;

			res = document->GetFrameRate(fps);
			FCM_CHECK;

			framesPerSec = (FCM::U_Int8)fps;
			FCM_CHECK;

			FCM::FCMListPtr libraryItems;
			res = document->GetLibraryItems(libraryItems.m_Ptr);
			if (FCM_FAILURE_CODE(res))
			{
				console.log("Failed to get library items");
				return res;
			}

			SharedWriter* writer;

			if (m_config.debug) {
				writer = new JSONWriter();
			}
			else {
				writer = new Writer();
			}

			res = writer->Init(GetCallback(), m_config);
			FCM_CHECK;

			res = m_resources.Init(writer, GetCallback());
			FCM_CHECK;

			res = ExportLibraryItems(libraryItems);
			FCM_CHECK;

			res = m_resources.Finalize();

			return res;
		}

		Result Publisher::Publish(DOM::PIFLADocument, DOM::PITimeline, const Exporter::Service::RANGE&, const PIFCMDictionary, const PIFCMDictionary) {
			return FCM_SERVICE_NOT_FOUND;
		};

		FCM::Result Publisher::ExportLibraryItems(FCM::FCMListPtr libraryItems) {
			U_Int32 itemCount = 0;
			Result res;

			res = libraryItems->Count(itemCount);
			FCM_CHECK;

			AutoPtr<FCM::IFCMUnknown> unknownService;
			res = GetCallback()->GetService(SRVCID_Core_Memory, unknownService.m_Ptr);
			AutoPtr<IFCMCalloc> callocService = unknownService;
			FCM_CHECK;

			for (FCM::U_Int32 i = 0; i < itemCount; i++)
			{
				FCM::StringRep16 itemName;
				std::string itemNameStr;

				AutoPtr<DOM::ILibraryItem> item = libraryItems[i];

				res = item->GetName(&itemName);
				FCM_CHECK;

				itemNameStr = Utils::ToString(itemName, GetCallback());

				AutoPtr<DOM::LibraryItem::IFolderItem> folderItem = item;
				if (folderItem)
				{
					FCM::FCMListPtr childrens;

					res = folderItem->GetChildren(childrens.m_Ptr);
					FCM_CHECK;

					// Export all its children
					res = ExportLibraryItems(childrens);
					FCM_CHECK;
				}
				else
				{
					AutoPtr<DOM::LibraryItem::ISymbolItem> symbolItem = item;
					if (!symbolItem) continue;

					AutoPtr<IFCMDictionary> dict;
					res = item->GetProperties(dict.m_Ptr);
					FCM_CHECK;

					std::string symbolType;
					Utils::ReadString(dict, kLibProp_SymbolType_DictKey, symbolType);

					if (symbolType != "MovieClip") continue;

					uint16_t symbolIdentifer;
					res = m_resources.GetIdentifer(itemNameStr, symbolIdentifer);
					FCM_CHECK;

					if (symbolIdentifer != UINT16_MAX) continue;

					res = m_resources.AddSymbol(itemNameStr, symbolItem, symbolIdentifer, true);
					FCM_CHECK;
				}

				callocService->Free((FCM::PVoid)itemName);
			}
			return FCM_SUCCESS;
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