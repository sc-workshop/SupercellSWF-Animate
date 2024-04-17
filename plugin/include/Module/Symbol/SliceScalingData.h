#pragma once

#include <vector>

#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/SliceElement.h"
#include "AnimateSDK/app/DOM/Utils/DOMTypes.h"
#include "AnimateSDK/app/DOM/ITimeline1.h"

// !!!!!!!!!!!!!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!! DANGER ZONE !!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!  Some magic numbers for 9slice Scaling Data !!!!!!
//   Here is present some offsets for CDocumentPage structure
// !!!!!!! that we can get from ITimeliine1 interface !!!!!!!!
// !!!!!!!!! and which is not officially documented !!!!!!!!!!
// !!!!!!!!!!!!!!!!!! (many thanks to IDA) !!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!! Windows:
// !!!!!!!!! Animate: 22.x, 23.x
// !!!!! GetScale9: 1624 bytes
// !!!!! GetScale9Rect: 1628 bytes

namespace sc
{
	namespace Adobe
	{
		class SliceScalingData
		{
		public:
			SliceScalingData();
			SliceScalingData(FCM::AutoPtr<DOM::ITimeline1> timeline);

		public:
			bool IsEnabled() const { return m_enabled; }
			const DOM::Utils::RECT& Guides() const { return m_guides; }

		private:
			bool m_enabled = false;
			DOM::Utils::RECT m_guides = { {0.0f, 0.0f}, {0.0f, 0.0f} };
		};
	}
}