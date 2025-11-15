#include "SlicedItem.h"

namespace sc
{
	namespace Adobe
	{
		SlicedItem::SlicedItem(
			Animate::Publisher::SymbolContext& context,
			wk::RawImageRef image, 
			const Animate::DOM::Utils::MATRIX2D& matrix,
			const wk::Point& translation,
			const Animate::DOM::Utils::RECT& guides
		) : BitmapItem(context), m_translation(translation)
		{
			m_matrix = matrix;
			m_image = image;
			m_guides = SlicedItem::RoundScalingGrid(guides);
		}

		wk::RectF SlicedItem::RoundScalingGrid(const Animate::DOM::Utils::RECT& guides)
		{
			return wk::RectF(
				std::min(guides.topLeft.x, guides.bottomRight.x), std::max(guides.topLeft.y, guides.bottomRight.y),
				std::max(guides.topLeft.x, guides.bottomRight.x), std::min(guides.topLeft.y, guides.bottomRight.y)
			);
		}
	}
}