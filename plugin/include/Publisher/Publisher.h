#pragma once

#include <FCMTypes.h>
#include <Utils/DOMTypes.h>
#include <PluginConfiguration.h>
#include <Publisher/IPublisher.h>

#include "DOM/ILibraryItem.h"
#include "DOM/LibraryItem/IFolderItem.h"
#include "DOM/LibraryItem/ISymbolItem.h"

#include <string>

#include "Version.h"
#include "io/Console.h"
#include "Ids.h"
#include "Utils.h"
#include "ResourcePublisher.h"
#include "Macros.h"

#include "Publisher/JSON/JSONWriter.h"

#include <filesystem>
namespace fs = std::filesystem;

using namespace FCM;
using namespace Publisher;
using namespace Adobe;
using namespace Exporter::Service;
using namespace DOM::Service::Tween;

#define PUBLISHER_OUTPUT "PublishSettings.SupercellSWF.output"
#define PUBLISHER_DEBUG "PublishSettings.SupercellSWF.debug"

namespace sc {
	namespace Adobe {
		class Publisher : public IPublisher, public FCMObjectBase
		{
			BEGIN_INTERFACE_MAP(Publisher, PLUGIN_VERSION)
				INTERFACE_ENTRY(IPublisher)
			END_INTERFACE_MAP

			std::string m_outputPath;
			Boolean m_debug = false;

			ResourcePublisher m_resources;

			Console console;

		public:
			Publisher() { }
			~Publisher() { }

			Result _FCMCALL Publish(
				DOM::PIFLADocument document,
				const PIFCMDictionary publishSettings,
				const PIFCMDictionary config) 
			{
				FCM::Result res;
				FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
				FCM::AutoPtr<FCM::IFCMCalloc> calloc;

				res = Init(publishSettings);
				if (FCM_FAILURE_CODE(res))
				{
					return res;
				}

				calloc = Utils::GetCallocService(GetCallback());
				ASSERT(calloc.m_Ptr != NULL);

				console.log("Creating output file : %s", m_outputPath.c_str());

				DOM::Utils::COLOR color;
				FCM::U_Int32 stageHeight;
				FCM::U_Int32 stageWidth;

				FCM::Double fps;
				FCM::U_Int8 framesPerSec;

				res = document->GetBackgroundColor(color);
				ASSERT(FCM_SUCCESS_CODE(res));

				res = document->GetStageHeight(stageHeight);
				ASSERT(FCM_SUCCESS_CODE(res));

				res = document->GetStageWidth(stageWidth);
				ASSERT(FCM_SUCCESS_CODE(res));

				res = document->GetFrameRate(fps);
				ASSERT(FCM_SUCCESS_CODE(res));

				framesPerSec = (FCM::U_Int8)fps;

				ASSERT(FCM_SUCCESS_CODE(res));

				FCM::FCMListPtr libraryItems;
				res = document->GetLibraryItems(libraryItems.m_Ptr);
				if (FCM_FAILURE_CODE(res))
				{
					console.log("Failed to get library items");
					return res;
				}

				fs::path outputPath = fs::path(m_outputPath);
				std::string outputFolder = outputPath.parent_path().string();
				std::string outputName = outputPath.filename().string();

				//if (m_debug) {
				JSONWriter writer;
				writer.Init(GetCallback(), outputFolder);
				//}
				m_resources.Init(&writer, GetCallback());

				ExportLibraryItems(libraryItems);

				res = m_resources.Finalize(outputName);
				FCM_CHECK;

				return FCM_SUCCESS;
			}

			Result _FCMCALL Publish(
				DOM::PIFLADocument document,
				DOM::PITimeline pTimeline,
				const Exporter::Service::RANGE& frameRange,
				const PIFCMDictionary publishSettings,
				const PIFCMDictionary config) {
				return FCM_SERVICE_NOT_FOUND;
			}

			FCM::Result _FCMCALL ClearCache() {
				return FCM_SUCCESS;
			}

		private:
			FCM::Result Init(const PIFCMDictionary config) {
				console.Init("Publisher", GetCallback());

				// Initialization of class members
				Utils::ReadString(config, PUBLISHER_OUTPUT, m_outputPath);
				if (m_outputPath.empty())
				{
					console.log("Failed to get output path");
					return FCM_INVALID_PARAM;
				}

				std::string debugMode = "0";
				Utils::ReadString(config, PUBLISHER_DEBUG, debugMode);
				if (!debugMode.empty() && debugMode == "1")
				{
					m_debug = true;
				}

				return FCM_SUCCESS;
			}

			FCM::Result ExportLibraryItems(FCM::FCMListPtr libraryItems) {
				FCM::U_Int32 count = 0;
				FCM::U_Int32 id = 0;
				FCM::Result res;

				ASSERT(libraryItems);

				res = libraryItems->Count(count);
				ASSERT(FCM_SUCCESS_CODE(res));

				FCM::AutoPtr<FCM::IFCMUnknown> pUnkCalloc;
				res = GetCallback()->GetService(SRVCID_Core_Memory, pUnkCalloc.m_Ptr);
				FCM_CHECK;
				AutoPtr<FCM::IFCMCalloc> callocService = pUnkCalloc;

				for (FCM::U_Int32 index = 0; index < count; index++)
				{
					FCM::StringRep16 LibItemName;
					std::string itemName;

					AutoPtr<IFCMDictionary> dict; // TODO: Remove?
					AutoPtr<DOM::ILibraryItem> item = libraryItems[index];

					res = item->GetName(&LibItemName);
					FCM_CHECK;

					itemName = Utils::ToString(LibItemName, GetCallback());

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

						res = item->GetProperties(dict.m_Ptr);
						FCM_CHECK;

						if (!symbolItem) continue;

						U_Int16 symbolIdentifer;
						m_resources.GetIdentifer(itemName, symbolIdentifer);

						if (symbolIdentifer != UINT16_MAX) continue;

						m_resources.AddSymbol(itemName, symbolItem);
					}

					callocService->Free((FCM::PVoid)LibItemName);
				}
				return FCM_SUCCESS;
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
					plugin.m_Ptr);

				{
					// Level 2 Dictionary
					AutoPtr<IFCMDictionary> category;
					res = plugin->AddLevel(
						(const FCM::StringRep8)kApplicationCategoryKey_Publisher,
						category.m_Ptr);

					{
						// Level 3 Dictionary

						// Add short name
						std::string str_name = PUBLISHER_NAME;
						res = category->Add(
							(const FCM::StringRep8)kApplicationCategoryKey_Name,
							kFCMDictType_StringRep8,
							(FCM::PVoid)str_name.c_str(),
							(FCM::U_Int32)str_name.length() + 1);

						// Add universal name - Used to refer to it from JSFL. Also, used in
						// error/warning messages.
						std::string str_uniname = PUBLISHER_UNIVERSAL_NAME;
						res = category->Add(
							(const FCM::StringRep8)kApplicationCategoryKey_UniversalName,
							kFCMDictType_StringRep8,
							(FCM::PVoid)str_uniname.c_str(),
							(FCM::U_Int32)str_uniname.length() + 1);

						std::string str_ui = PUBLISH_SETTINGS_UI_ID;
						res = category->Add(
							(const FCM::StringRep8)kApplicationPublisherKey_UI,
							kFCMDictType_StringRep8,
							(FCM::PVoid)str_ui.c_str(),
							(FCM::U_Int32)str_ui.length() + 1);

						AutoPtr<IFCMDictionary> pDocs;
						res = category->AddLevel((const FCM::StringRep8)kApplicationPublisherKey_TargetDocs, pDocs.m_Ptr);

						{
							// Level 4 Dictionary
							std::string empytString = "";
							res = pDocs->Add(
								(const FCM::StringRep8)Utils::ToString(docId).c_str(),
								kFCMDictType_StringRep8,
								(FCM::PVoid)empytString.c_str(),
								(FCM::U_Int32)empytString.length() + 1);
						}
					}
				}
			}

			return res;
		}
	}
}