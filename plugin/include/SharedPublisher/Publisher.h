#pragma once

#include <FCMTypes.h>
#include <Utils/DOMTypes.h>
#include <PluginConfiguration.h>
#include <Publisher/IPublisher.h>

#include <string>

#include "Version.h"
#include "io/Console.h"

using namespace Publisher;
using namespace Exporter::Service;

#define PUBLISHER_OUTPUT "PublishSettings.SupercellSWF.output"
#define PUBLISHER_MODE "PublishSettings.SupercellSWF.mode"

namespace SupercellSWF {
    enum class PublisherMode {
        JSON
    };

    class SharedPublisher : public IPublisher, public FCMObjectBase
    {
        BEGIN_INTERFACE_MAP(SharedPublisher, PLUGIN_VERSION)
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

        SharedPublisher();

        ~SharedPublisher();

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
        AutoPtr<IResourcePalette> m_resourcePalette;

        Console console;
    };

    FCM::Result RegisterPublisher(PIFCMDictionary plugins, FCM::FCMCLSID docId);
}