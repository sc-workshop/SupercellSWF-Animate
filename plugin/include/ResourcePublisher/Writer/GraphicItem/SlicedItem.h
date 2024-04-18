#pragma once

#include "SpriteItem.h"
#include "math/rect.h"
#include "generic/ref.h"

namespace sc
{
	namespace Adobe
	{
		class SlicedItem : public SpriteItem
		{
		public:
			SlicedItem(Ref<cv::Mat> image, const DOM::Utils::MATRIX2D& matrix, const DOM::Utils::RECT guides);
			virtual ~SlicedItem() = default;

		public:
			virtual bool IsSliced() const
			{
				return true;
			}

			const Rect<int32_t>& Guides()
			{
				return m_guides;
			}

		private:
			Rect<int32_t> m_guides;
		};
	}
}