#include "SharedPublisher/Publisher.h"


#include <ApplicationFCMPublicIDs.h>
#include <LibraryItem/IFolderItem.h>
#include <LibraryItem/IFontItem.h>
#include <LibraryItem/ISymbolItem.h>
#include <ILibraryItem.h>

#include <Exporter/Service/IResourcePalette.h>
#include <Exporter/Service/ITimelineBuilder2.h>
#include <Exporter/Service/ITimelineBuilderFactory.h>

#include "SharedPublisher/ResourcePalette.h"
#include "SharedPublisher/TimelineBuilder.h"
#include "SharedPublisher/TimelineBuilderFactory.h"

#include "Writers/Base/OutputWriter.h"
#include "Writers/Base/TimelineWriter.h"
#include "Writers/JSON/JSONOutputWriter.h"
#include "Writers/JSON/JSONTimelineWriter.h"

#include <algorithm>

#include "Ids.h"
#include "Utils.h"

#define SCSWF_PUBLISHER_OUTPUT "PublishSettings.SupercellSWF.output"

namespace SupercellSWF {
    ModulePublisher::ModulePublisher()
    {

    }

    ModulePublisher::~ModulePublisher()
    {

    }


    FCM::Result ModulePublisher::Publish(
        DOM::PIFLADocument pFlaDocument,
        const PIFCMDictionary pDictPublishSettings,
        const PIFCMDictionary pDictConfig)
    {
        return Export(pFlaDocument, pDictPublishSettings, pDictConfig);
    }

    // This function will be currently called in "Test-Scene" workflow. 
    // In future, it might be called in other workflows as well. 
    FCM::Result ModulePublisher::Publish(
        DOM::PIFLADocument pFlaDocument,
        DOM::PITimeline pTimeline,
        const Exporter::Service::RANGE& frameRange,
        const PIFCMDictionary pDictPublishSettings,
        const PIFCMDictionary pDictConfig)
    {
        return FCM_SERVICE_NOT_FOUND;
    }


    FCM::Result ModulePublisher::Export(
        DOM::PIFLADocument pFlaDocument,
        const PIFCMDictionary pDictPublishSettings,
        const PIFCMDictionary pDictConfig)
    {
        std::string outFile;
        FCM::Result res;
        FCM::FCMGUID guid;
        FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
        FCM::AutoPtr<FCM::IFCMCalloc> pCalloc;
        ///////////////////////
        Init();

        pCalloc = SupercellSWF::Utils::GetCallocService(GetCallback());
        ASSERT(pCalloc.m_Ptr != NULL);

        res = pFlaDocument->GetTypeId(guid);
        ASSERT(FCM_SUCCESS_CODE(res));

        std::string pub_guid = Utils::ToString(guid);
        Utils::Trace(GetCallback(), "Publish. GUID: %s\n",
            pub_guid.c_str());

        res = GetOutputFileName(pFlaDocument, pDictPublishSettings, outFile);
        if (FCM_FAILURE_CODE(res))
        {
            // FLA is untitled. Ideally, we should use a temporary location for output generation.
            // However, for now, we report an error.
            Utils::Trace(GetCallback(), "Failed to publish. Either save the FLA or provide output path in publish settings.\n");
            return res;
        }

        Utils::Trace(GetCallback(), "Creating output file : %s\n", outFile.c_str());

        DOM::Utils::COLOR color;
        FCM::U_Int32 stageHeight;
        FCM::U_Int32 stageWidth;
        FCM::Double fps;
        FCM::U_Int32 framesPerSec;
        AutoPtr<ITimelineBuilderFactory> pTimelineBuilderFactory;
        FCM::FCMListPtr pTimelineList;
        FCM::U_Int32 timelineCount;

        // Create a output writer
        std::shared_ptr<OutputWriter> pOutputWriter(new JSONOutputWriter(GetCallback()));
        if (pOutputWriter.get() == NULL)
        {
            return FCM_MEM_NOT_AVAILABLE;
        }

        // Start output
        pOutputWriter->StartOutput(outFile);

        // Create a Timeline Builder Factory for the root timeline of the document
        res = GetCallback()->CreateInstance(
            NULL,
            CLSID_TimelineBuilderFactory,
            IID_ITimelineBuilderFactory,
            (void**)&pTimelineBuilderFactory);
        if (FCM_FAILURE_CODE(res))
        {
            return res;
        }

        (static_cast<TimelineBuilderFactory*>(pTimelineBuilderFactory.m_Ptr))->Init(
            pOutputWriter.get());

        ResourcePalette* pResPalette = static_cast<ResourcePalette*>(m_pResourcePalette.m_Ptr);
        pResPalette->Clear();
        pResPalette->Init(pOutputWriter.get());

        res = pFlaDocument->GetBackgroundColor(color);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = pFlaDocument->GetStageHeight(stageHeight);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = pFlaDocument->GetStageWidth(stageWidth);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = pFlaDocument->GetFrameRate(fps);
        ASSERT(FCM_SUCCESS_CODE(res));

        framesPerSec = (FCM::U_Int32)fps;

        res = pOutputWriter->StartDocument(color, stageHeight, stageWidth, framesPerSec);
        ASSERT(FCM_SUCCESS_CODE(res));

        // Get all the timelines for the document
        res = pFlaDocument->GetTimelines(pTimelineList.m_Ptr);
        if (FCM_FAILURE_CODE(res))
        {
            return res;
        }

        res = pTimelineList->Count(timelineCount);
        if (FCM_FAILURE_CODE(res))
        {
            return res;
        }

        // Generate frame commands for each timeline
        for (FCM::U_Int32 i = 0; i < timelineCount; i++)
        {
            Exporter::Service::RANGE range;
            AutoPtr<ITimelineBuilder> pTimelineBuilder;
            TimelineWriter* pTimelineWriter;

            AutoPtr<DOM::ITimeline> timeline = pTimelineList[i];

            range.min = 0;
            res = timeline->GetMaxFrameCount(range.max);
            if (FCM_FAILURE_CODE(res))
            {
                return res;
            }

            range.max--;

            // Generate frame commands
            res = m_frameCmdGeneratorService->GenerateFrameCommands(
                timeline,
                range,
                pDictPublishSettings,
                m_pResourcePalette,
                pTimelineBuilderFactory,
                pTimelineBuilder.m_Ptr);

            if (FCM_FAILURE_CODE(res))
            {
                return res;
            }

            ((TimelineBuilder*)pTimelineBuilder.m_Ptr)->Build(0, NULL, &pTimelineWriter);
        }

        res = pOutputWriter->EndDocument();
        ASSERT(FCM_SUCCESS_CODE(res));

        res = pOutputWriter->EndOutput();
        ASSERT(FCM_SUCCESS_CODE(res));

        // Export the library items with linkages
        FCM::FCMListPtr pLibraryItemList;
        res = pFlaDocument->GetLibraryItems(pLibraryItemList.m_Ptr);
        if (FCM_FAILURE_CODE(res))
        {
            return res;
        }

        ExportLibraryItems(pLibraryItemList);

        return FCM_SUCCESS;
    }


    FCM::Result ModulePublisher::ClearCache()
    {
        if (m_pResourcePalette)
        {
            ResourcePalette* pResPalette = static_cast<ResourcePalette*>(m_pResourcePalette.m_Ptr);

            pResPalette->Clear();
        }
        return FCM_SUCCESS;
    }


    FCM::Result ModulePublisher::GetOutputFileName(
        DOM::PIFLADocument pFlaDocument,
        const PIFCMDictionary pDictPublishSettings,
        std::string& outFile)
    {
        FCM::Result res = FCM_SUCCESS;
        FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
        FCM::AutoPtr<FCM::IFCMCalloc> pCalloc;

        pCalloc = SupercellSWF::Utils::GetCallocService(GetCallback());
        ASSERT(pCalloc.m_Ptr != NULL);

        // Read the output file name from the publish settings
        Utils::ReadString(pDictPublishSettings, SCSWF_PUBLISHER_OUTPUT, outFile);
        if (outFile.empty())
        {
            res = FCM_INVALID_PARAM;
        }

        return res;
    }


    FCM::Boolean ModulePublisher::IsPreviewNeeded(const PIFCMDictionary pDictConfig)
    {
        FCM::Boolean found;
        std::string previewNeeded;
        FCM::Boolean res;

        found = Utils::ReadString(pDictConfig, (FCM::StringRep8)kPublishSettingsKey_PreviewNeeded, previewNeeded);

        res = true;
        if (found)
        {
            if (previewNeeded == "true")
            {
                res = true;
            }
            else
            {
                res = false;
            }
        }
        return res;
    }


    FCM::Result ModulePublisher::ShowPreview(const std::string& outFile)
    {
        FCM::Result res = FCM_SUCCESS;

        return res;
    }


    FCM::Result ModulePublisher::Init()
    {
        FCM::Result res = FCM_SUCCESS;;
        FCM::AutoPtr<FCM::IFCMUnknown> pUnk;

        if (!m_frameCmdGeneratorService)
        {
            // Get the frame command generator service
            res = GetCallback()->GetService(Exporter::Service::EXPORTER_FRAME_CMD_GENERATOR_SERVICE, pUnk.m_Ptr);
            m_frameCmdGeneratorService = pUnk;
        }

        if (!m_pResourcePalette)
        {
            // Create a Resource Palette
            res = GetCallback()->CreateInstance(NULL, CLSID_ResourcePalette, IID_IResourcePalette, (void**)&m_pResourcePalette);
            ASSERT(FCM_SUCCESS_CODE(res));
        }

        return res;
    }


    //
    // Note: This function is NOT completely implemented but provides guidelines 
    // on how this can be possibly done.      
    //
    FCM::Result ModulePublisher::ExportLibraryItems(FCM::FCMListPtr pLibraryItemList)
    {
        FCM::U_Int32 count = 0;
        FCM::Result res;


        ASSERT(pLibraryItemList);

        res = pLibraryItemList->Count(count);
        ASSERT(FCM_SUCCESS_CODE(res));

        FCM::AutoPtr<FCM::IFCMUnknown> pUnkCalloc;
        res = GetCallback()->GetService(SRVCID_Core_Memory, pUnkCalloc.m_Ptr);
        AutoPtr<FCM::IFCMCalloc> callocService = pUnkCalloc;

        for (FCM::U_Int32 index = 0; index < count; index++)
        {
            FCM::StringRep16 pLibItemName = NULL;
            std::string libItemName;
            AutoPtr<IFCMDictionary> pDict;
            AutoPtr<DOM::ILibraryItem> pLibItem = pLibraryItemList[index];

            res = pLibItem->GetName(&pLibItemName);
            ASSERT(FCM_SUCCESS_CODE(res));
            libItemName = Utils::ToString(pLibItemName, GetCallback());

            AutoPtr<DOM::LibraryItem::IFolderItem> pFolderItem = pLibItem;
            if (pFolderItem)
            {
                FCM::FCMListPtr pChildren;

                res = pFolderItem->GetChildren(pChildren.m_Ptr);
                ASSERT(FCM_SUCCESS_CODE(res));

                // Export all its children
                res = ExportLibraryItems(pChildren);
                ASSERT(FCM_SUCCESS_CODE(res));
            }
            else
            {
                FCM::FCMDictRecTypeID type;
                FCM::U_Int32 valLen;
                AutoPtr<DOM::LibraryItem::IFontItem> pFontItem = pLibItem;
                AutoPtr<DOM::LibraryItem::ISymbolItem> pSymbolItem = pLibItem;
                AutoPtr<DOM::LibraryItem::IMediaItem> pMediaItem = pLibItem;

                res = pLibItem->GetProperties(pDict.m_Ptr);
                ASSERT(FCM_SUCCESS_CODE(res));

                res = pDict->GetInfo(kLibProp_LinkageClass_DictKey,
                    type, valLen);

                if (FCM_SUCCESS_CODE(res))
                {
                    FCM::Boolean hasResource;
                    ResourcePalette* pResPalette = static_cast<ResourcePalette*>(m_pResourcePalette.m_Ptr);

                    // Library Item has linkage identifer

                    if (pSymbolItem)
                    {
                        //
                        // Check if it has been exported already by comparing names of resources 
                        // already exported from the timelines.
                        //
                        res = pResPalette->HasResource(libItemName, hasResource);
                        if (!hasResource)
                        {
                            // Resource is not yet exported. Export it using 
                            // FrameCommandGenerator::GenerateFrameCommands
                        }
                    }
                    else if (pMediaItem)
                    {
                        //
                        // Check if it has been exported already by comparing names of resources 
                        // already exported from the timelines.
                        //
                        res = pResPalette->HasResource(libItemName, hasResource);
                        if (!hasResource)
                        {
                            // Resource is not yet exported. Export it.

                            // Depending on bitmap/sound, export it.
                        }
                    }
                    else if (pFontItem)
                    {
                        // Use the font name to check if already exported.

                        // Use IFontTableGeneratorService::CreateFontTableForFontItem() to create 
                        // a font table and then export it.
                    }
                }
            }

            callocService->Free((FCM::PVoid)pLibItemName);
        }
        return FCM_SUCCESS;
    }

    FCM::Result RegisterPublisher(PIFCMDictionary pPlugins, FCM::FCMCLSID docId)
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
            AutoPtr<IFCMDictionary> pPlugin;
            res = pPlugins->AddLevel(
                (const FCM::StringRep8)Utils::ToString(CLSID_Publisher).c_str(),
                pPlugin.m_Ptr);

            {
                // Level 2 Dictionary
                AutoPtr<IFCMDictionary> pCategory;
                res = pPlugin->AddLevel(
                    (const FCM::StringRep8)kApplicationCategoryKey_Publisher,
                    pCategory.m_Ptr);

                {
                    // Level 3 Dictionary

                    // Add short name
                    std::string str_name = PUBLISHER_NAME;
                    res = pCategory->Add(
                        (const FCM::StringRep8)kApplicationCategoryKey_Name,
                        kFCMDictType_StringRep8,
                        (FCM::PVoid)str_name.c_str(),
                        (FCM::U_Int32)str_name.length() + 1);

                    // Add universal name - Used to refer to it from JSFL. Also, used in 
                    // error/warning messages.
                    std::string str_uniname = PUBLISHER_UNIVERSAL_NAME;
                    res = pCategory->Add(
                        (const FCM::StringRep8)kApplicationCategoryKey_UniversalName,
                        kFCMDictType_StringRep8,
                        (FCM::PVoid)str_uniname.c_str(),
                        (FCM::U_Int32)str_uniname.length() + 1);

                    std::string str_ui = PUBLISH_SETTINGS_UI_ID;
                    res = pCategory->Add(
                        (const FCM::StringRep8)kApplicationPublisherKey_UI,
                        kFCMDictType_StringRep8,
                        (FCM::PVoid)str_ui.c_str(),
                        (FCM::U_Int32)str_ui.length() + 1);

                    AutoPtr<IFCMDictionary> pDocs;
                    res = pCategory->AddLevel((const FCM::StringRep8)kApplicationPublisherKey_TargetDocs, pDocs.m_Ptr);

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