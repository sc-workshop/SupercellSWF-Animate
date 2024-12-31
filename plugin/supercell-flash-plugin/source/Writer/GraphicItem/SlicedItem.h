#pragma once

#include "SpriteItem.h"
#include "core/math/rect.h"
#include "core/math/point.h"

namespace sc
{
	namespace Adobe
	{
		class SlicedItem : public BitmapItem
		{
		public:
			SlicedItem(
				Animate::Publisher::SymbolContext& context,
				wk::Ref<cv::Mat> image,
				const wk::Point& translation,
				const Animate::Publisher::Slice9Element& element
			);

		public:
			const wk::Point& Translation() const
			{
				return m_translation;
			}

			const wk::Rect& Guides() const
			{
				return m_guides;
			}

			virtual bool Is9Sliced() const
			{
				return true;
			}

			static wk::Rect RoundScalingGrid(const Animate::DOM::Utils::RECT&);

		private:
			wk::Point m_translation;
			wk::Rect m_guides;
		};
	}
}