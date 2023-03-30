#pragma once

#include <FCMTypes.h>
#include <Utils/DOMTypes.h>
#include <PluginConfiguration.h>
#include <Publisher/IPublisher.h>

#include <string>

using namespace Publisher;
using namespace Exporter::Service;

namespace SupercellSWF {
    class ModulePublisher : public IPublisher, public FCMObjectBase
    {
        BEGIN_INTERFACE_MAP(ModulePublisher, PLUGIN_VERSION)
            INTERFACE_ENTRY(IPublisher)
        END_INTERFACE_MAP

    public:

        FCM::Result _FCMCALL Publish(
            DOM::PIFLADocument pFlaDocument,
            const PIFCMDictionary pDictPublishSettings,
            const PIFCMDictionary pDictConfig);

        FCM::Result _FCMCALL Publish(
            DOM::PIFLADocument pFlaDocument,
            DOM::PITimeline pTimeline,
            const Exporter::Service::RANGE& frameRange,
            const PIFCMDictionary pDictPublishSettings,
            const PIFCMDictionary pDictConfig);

        FCM::Result _FCMCALL ClearCache();

        ModulePublisher();

        ~ModulePublisher();

    private:
        FCM::Result GetOutputFileName(
            DOM::PIFLADocument pFlaDocument,
            const PIFCMDictionary pDictPublishSettings,
            std::string& outFile);

        FCM::Result Export(
            DOM::PIFLADocument pFlaDocument,
            const PIFCMDictionary pDictPublishSettings,
            const PIFCMDictionary pDictConfig);

        FCM::Boolean IsPreviewNeeded(const PIFCMDictionary pDictConfig);

        FCM::Result Init();

        FCM::Result ShowPreview(const std::string& outFile);

        FCM::Result ExportLibraryItems(FCM::FCMListPtr pLibraryItemList);

    private:

        AutoPtr<IFrameCommandGenerator> m_frameCmdGeneratorService;
        AutoPtr<IResourcePalette> m_pResourcePalette;
    };

    FCM::Result RegisterPublisher(PIFCMDictionary pPlugins, FCM::FCMCLSID docId);
}