#pragma once

#include "SpriteItem.h"
#include "core/math/rect.h"

namespace sc
{
	namespace Adobe
	{
		class SlicedItem : public SpriteItem
		{
		public:

		public:
			SlicedItem(wk::Ref<cv::Mat> image, const Animate::DOM::Utils::MATRIX2D& matrix, const Animate::DOM::Utils::RECT guides);
			virtual ~SlicedItem() = default;

		public:
			virtual bool IsSliced() const
			{
				return true;
			}

			const wk::Rect& Guides() const
			{
				return m_guides;
			}

		private:
			wk::Rect m_guides;
		};
	}
}