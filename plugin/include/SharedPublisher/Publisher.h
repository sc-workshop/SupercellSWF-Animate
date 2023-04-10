#pragma once

#include <FCMTypes.h>
#include <Utils/DOMTypes.h>
#include <PluginConfiguration.h>
#include <Publisher/IPublisher.h>

#include <string>

#include "Version.h"

using namespace Publisher;
using namespace Exporter::Service;

#define PUBLISHER_OUTPUT "PublishSettings.SupercellSWF.output"
#define PUBLISHER_MODE "PublishSettings.SupercellSWF.mode"

namespace SupercellSWF {
    enum class PublisherMode {
        JSON
    };

    class ModulePublisher : public IPublisher, public FCMObjectBase
    {
        BEGIN_INTERFACE_MAP(ModulePublisher, PLUGIN_VERSION)
            INTERFACE_ENTRY(IPublisher)
        END_INTERFACE_MAP

    public:

        FCM::Result _FCMCALL Publish(
            DOM::PIFLADocument document,
            const PIFCMDictionary publishSettings,
            const PIFCMDictionary config);

        FCM::Result _FCMCALL Publish(
            DOM::PIFLADocument document,
            DOM::PITimeline pTimeline,
            const Exporter::Service::RANGE& frameRange,
            const PIFCMDictionary publishSettings,
            const PIFCMDictionary config);

        FCM::Result _FCMCALL ClearCache();

        ModulePublisher();

        ~ModulePublisher();

    private:
        FCM::Result Export(
            DOM::PIFLADocument document,
            const PIFCMDictionary publishSettings,
            const PIFCMDictionary config);

        FCM::Boolean IsPreviewNeeded(const PIFCMDictionary config);

        FCM::Result Init(const PIFCMDictionary config);

        FCM::Result ShowPreview(const std::string& outFile);

        FCM::Result ExportLibraryItems(FCM::FCMListPtr libraryItems);

    private:
        std::string m_outputPath;
        PublisherMode m_publishMode;

        AutoPtr<IFrameCommandGenerator> m_frameCmdGeneratorService;
        AutoPtr<IResourcePalette> m_pResourcePalette;
    };

    FCM::Result RegisterPublisher(PIFCMDictionary pPlugins, FCM::FCMCLSID docId);
}