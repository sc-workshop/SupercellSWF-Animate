#include "SlicedItem.h"

namespace sc
{
	namespace Adobe
	{
		SlicedItem::SlicedItem(
			Animate::Publisher::SymbolContext& context,
			wk::Ref<cv::Mat> image, 
			const wk::Point& translation,
			const Animate::Publisher::Slice9Element& element
		) : BitmapItem(context), m_translation(translation)
		{
			m_guides = SlicedItem::RoundScalingGrid(element.Guides());
		}

		wk::Rect SlicedItem::RoundScalingGrid(const Animate::DOM::Utils::RECT& guides)
		{
			return wk::Rect(
				(int)std::ceil(std::min(guides.topLeft.x, guides.bottomRight.x)), (int)std::ceil(std::max(guides.topLeft.y, guides.bottomRight.y)),
				(int)std::ceil(std::max(guides.topLeft.x, guides.bottomRight.x)), (int)std::ceil(std::min(guides.topLeft.y, guides.bottomRight.y))
			);
		}
	}
}