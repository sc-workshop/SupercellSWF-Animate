#include "SharedPublisher/TimelineBuilderFactory.h"

#include "SharedPublisher/TimelineBuilder.h"

namespace SupercellSWF {
    TimelineBuilderFactory::TimelineBuilderFactory()
    {
    }

    TimelineBuilderFactory::~TimelineBuilderFactory()
    {
    }

    FCM::Result TimelineBuilderFactory::CreateTimelineBuilder(PITimelineBuilder& pTimelineBuilder)
    {
        FCM::Result res = GetCallback()->CreateInstance(NULL, CLSID_TimelineBuilder, IID_ITIMELINE_BUILDER_2, (void**)&pTimelineBuilder);

        TimelineBuilder* pTimeline = static_cast<TimelineBuilder*>(pTimelineBuilder);

        pTimeline->Init(m_pOutputWriter);

        return res;
    }

    void TimelineBuilderFactory::Init(OutputWriter* pOutputWriter)
    {
        m_pOutputWriter = pOutputWriter;
    }
}