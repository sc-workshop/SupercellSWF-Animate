#include "SharedPublisher/TimelineBuilderFactory.h"

#include "SharedPublisher/TimelineBuilder.h"
#include "Ids.h"

namespace SupercellSWF {
    TimelineBuilderFactory::TimelineBuilderFactory()
    {
    }

    TimelineBuilderFactory::~TimelineBuilderFactory()
    {
    }

    FCM::Result TimelineBuilderFactory::CreateTimelineBuilder(PITimelineBuilder& timelineBuilder)
    {
        FCM::Result res = GetCallback()->CreateInstance(NULL, CLSID_TimelineBuilder, IID_ITIMELINE_BUILDER_2, (void**)&timelineBuilder);

        TimelineBuilder* pTimeline = static_cast<TimelineBuilder*>(timelineBuilder);

        pTimeline->Init(m_outputWriter, m_outputWriter->createTimelineWriter(GetCallback()) );

        return res;
    }

    void TimelineBuilderFactory::Init(OutputWriter* outputWriter)
    {
        m_outputWriter = outputWriter;
    }
}