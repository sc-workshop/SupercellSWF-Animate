#pragma once

#include <FCMTypes.h>
#include <Exporter/Service/ITimelineBuilderFactory.h>
#include <Exporter/Service/IResourcePalette.h>

#include "PluginConfiguration.h"
#include "Writers/Base/OutputWriter.h"
#include "Writers/Base/TimelineWriter.h"

#include "Version.h"

using namespace Exporter::Service;

namespace SupercellSWF {
    class TimelineBuilderFactory : public ITimelineBuilderFactory, public FCMObjectBase
    {
    public:

        BEGIN_INTERFACE_MAP(TimelineBuilderFactory, PLUGIN_VERSION)
            INTERFACE_ENTRY(ITimelineBuilderFactory)
        END_INTERFACE_MAP

        virtual FCM::Result _FCMCALL CreateTimelineBuilder(PITimelineBuilder& pTimelineBuilder);

        TimelineBuilderFactory();

        ~TimelineBuilderFactory();

        void Init(OutputWriter* outputWriter);

    private:

        OutputWriter* m_outputWriter;
    };
}