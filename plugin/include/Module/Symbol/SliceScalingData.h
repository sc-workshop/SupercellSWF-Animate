#pragma once

#include <vector>

#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/SliceElement.h"
#include "AnimateSDK/app/DOM/Utils/DOMTypes.h"
#include "AnimateSDK/app/DOM/ITimeline1.h"

namespace sc
{
	namespace Adobe
	{
		class SliceScalingData
		{
		public:
			SliceScalingData() {}

			SliceScalingData(FCM::AutoPtr<DOM::ITimeline1> timeline)
			{
				CDocumentPage* page = timeline->GetDocPage();
				m_enabled = page->GetScale9();
				if (m_enabled)
				{
					page->GetScale9Rect(m_guides);
				}
			}

		public:
			bool IsEnabled() const { return m_enabled; }
			const DOM::Utils::RECT& Guides() const { return m_guides; }

		private:
			bool m_enabled = false;
			DOM::Utils::RECT m_guides = { {0.0f, 0.0f}, {0.0f, 0.0f} };
		};
	}
}