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

#include <algorithm>

#include "Ids.h"
#include "Utils.h"
    
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
        const PIFCMDictionary config)
    {
        return Export(pFlaDocument, pDictPublishSettings, config);
    }

    FCM::Result ModulePublisher::Publish(
        DOM::PIFLADocument pFlaDocument,
        DOM::PITimeline pTimeline,
        const Exporter::Service::RANGE& frameRange,
        const PIFCMDictionary pDictPublishSettings,
        const PIFCMDictionary config)
    {
        return FCM_SERVICE_NOT_FOUND;
    }


    FCM::Result ModulePublisher::Export(
        DOM::PIFLADocument document,
        const PIFCMDictionary publishSettings,
        const PIFCMDictionary config)
    {
        FCM::Result res;
        FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
        FCM::AutoPtr<FCM::IFCMCalloc> pCalloc;

        res = Init(publishSettings);
        if (FCM_FAILURE_CODE(res))
        {
            return res;
        }

        pCalloc = SupercellSWF::Utils::GetCallocService(GetCallback());
        ASSERT(pCalloc.m_Ptr != NULL);

        Utils::Trace(GetCallback(), "[Publisher] Creating output file : %s\n", m_outputPath.c_str());

        DOM::Utils::COLOR color;
        FCM::U_Int32 stageHeight;
        FCM::U_Int32 stageWidth; // TODO remove unused

        FCM::Double fps;
        FCM::U_Int32 framesPerSec;
        AutoPtr<ITimelineBuilderFactory> timelineBuilderFactory;

        FCM::FCMListPtr pTimelineList; // TODO remove this
        FCM::U_Int32 timelineCount;
        
        // Create a output writer
        std::shared_ptr<OutputWriter> outputWriter;
        switch (m_publishMode)
        {
        case PublisherMode::JSON:
            outputWriter = std::shared_ptr<OutputWriter>(new JSONOutputWriter(GetCallback()));
            /* code */
            break;
        }

        if (outputWriter.get() == NULL)
        {
            return FCM_MEM_NOT_AVAILABLE;
        }

        // Start output
        outputWriter->StartOutput(m_outputPath);

        // Create a Timeline Builder Factory for the root timeline of the document
        res = GetCallback()->CreateInstance(
            NULL,
            CLSID_TimelineBuilderFactory,
            IID_ITimelineBuilderFactory,
            (void**)&timelineBuilderFactory);
        if (FCM_FAILURE_CODE(res))
        {
            return res;
        }

        (static_cast<TimelineBuilderFactory*>(timelineBuilderFactory.m_Ptr))->Init(
            outputWriter.get());

        ResourcePalette* pResPalette = static_cast<ResourcePalette*>(m_pResourcePalette.m_Ptr);
        pResPalette->Clear();
        pResPalette->Init(outputWriter.get());

        res = document->GetBackgroundColor(color);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = document->GetStageHeight(stageHeight);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = document->GetStageWidth(stageWidth);
        ASSERT(FCM_SUCCESS_CODE(res));

        res = document->GetFrameRate(fps);
        ASSERT(FCM_SUCCESS_CODE(res));

        framesPerSec = (FCM::U_Int32)fps;

        res = outputWriter->StartDocument(color, stageHeight, stageWidth, framesPerSec);
        ASSERT(FCM_SUCCESS_CODE(res));

        // Get all the timelines for the document
        res = document->GetTimelines(pTimelineList.m_Ptr);
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
        for (FCM::U_Int32 i = 0; i < timelineCount; i++) // TODO move to library items exporting
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
                publishSettings,
                m_pResourcePalette,
                timelineBuilderFactory,
                pTimelineBuilder.m_Ptr);

            if (FCM_FAILURE_CODE(res))
            {
                return res;
            }

            ((TimelineBuilder*)pTimelineBuilder.m_Ptr)->Build(0, NULL, &pTimelineWriter);
        }

        res = outputWriter->EndDocument();
        ASSERT(FCM_SUCCESS_CODE(res));

        res = outputWriter->EndOutput();
        ASSERT(FCM_SUCCESS_CODE(res));

        // Export the library items with linkages
        FCM::FCMListPtr pLibraryItemList;
        res = document->GetLibraryItems(pLibraryItemList.m_Ptr);
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

    FCM::Boolean ModulePublisher::IsPreviewNeeded(const PIFCMDictionary config)
    {
        FCM::Boolean found;
        std::string previewNeeded;
        FCM::Boolean res;

        found = Utils::ReadString(config, (FCM::StringRep8)kPublishSettingsKey_PreviewNeeded, previewNeeded);

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


    FCM::Result ModulePublisher::Init(const PIFCMDictionary config)
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

        // Initialization of class members

        Utils::ReadString(config, PUBLISHER_OUTPUT, m_outputPath);
        if (m_outputPath.empty())
        {
            Utils::Trace(GetCallback(), "[PublisherSettings] Failed to get output path.\n");
            return FCM_INVALID_PARAM;
        }

        std::string modeName;
        Utils::ReadString(config, PUBLISHER_MODE, modeName);
        if (modeName == "JSON") {
            m_publishMode = PublisherMode::JSON;
        } else {
            Utils::Trace(GetCallback(), "[PublisherSettings] Failed to get publisher mode. Default mode will be used - JSON.\n");
            m_publishMode = PublisherMode::JSON;
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