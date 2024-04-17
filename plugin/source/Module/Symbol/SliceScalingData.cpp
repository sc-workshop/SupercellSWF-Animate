#include "Module/Symbol/SliceScalingData.h"

namespace sc
{
	namespace Adobe
	{
		SliceScalingData::SliceScalingData() {}

		SliceScalingData::SliceScalingData(FCM::AutoPtr<DOM::ITimeline1> timeline)
		{
			const size_t is_enabled_offset = 1624;
			const size_t grid_data_offset = 1628;

			uint8_t* page = (uint8_t*)timeline->GetDocPage();
			m_enabled = *((bool*)page + is_enabled_offset);

			if (m_enabled)
			{
				uint8_t* grid = page + grid_data_offset;

				const size_t stride = sizeof(int32_t);

				// Some ugly uh methods to get values
				int32_t left = *(int32_t*)grid;
				int32_t right = *(int32_t*)(grid + stride);

				int32_t top = *(int32_t*)(grid + (stride * 2));
				int32_t bottom = *(int32_t*)(grid + (stride * 3));

				m_guides.topLeft.x = (float)top / 20.0f;
				m_guides.topLeft.y = (float)left / 20.0f;

				m_guides.bottomRight.x = (float)bottom / 20.0f;
				m_guides.bottomRight.y = (float)right / 20.0f;
			}
		}
	}
}