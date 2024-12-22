#include "SlicedItem.h"

namespace sc
{
	namespace Adobe
	{
		SlicedItem::SlicedItem(wk::Ref<cv::Mat> image, const Animate::DOM::Utils::MATRIX2D& matrix, const Animate::DOM::Utils::RECT guides, const wk::Point& translation) :
			SpriteItem(image, matrix), m_translation(translation)
		{
			m_guides = SlicedItem::RoundScalingGrid(guides);
		}

		wk::Rect SlicedItem::RoundScalingGrid(const Animate::DOM::Utils::RECT& guides)
		{
			return wk::Rect(
				std::ceil(std::min(guides.topLeft.x, guides.bottomRight.x)), std::ceil(std::max(guides.topLeft.y, guides.bottomRight.y)),
				std::ceil(std::max(guides.topLeft.x, guides.bottomRight.x)), std::ceil(std::min(guides.topLeft.y, guides.bottomRight.y))
			);
		}
	}
}