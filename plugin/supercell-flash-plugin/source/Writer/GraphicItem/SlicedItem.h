#pragma once

#include "SpriteItem.h"
#include "core/math/rect.h"
#include "core/math/point.h"

namespace sc
{
	namespace Adobe
	{
		class SlicedItem : public SpriteItem
		{
		public:

		public:
			SlicedItem(wk::Ref<cv::Mat> image, const Animate::DOM::Utils::MATRIX2D& matrix, const Animate::DOM::Utils::RECT guides, const wk::Point& translation);
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

			const wk::Point& translation() const
			{
				return m_translation;
			}

			static wk::Rect RoundScalingGrid(const Animate::DOM::Utils::RECT&);

		private:
			wk::Rect m_guides;
			wk::Point m_translation;
		};
	}
}