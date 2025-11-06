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

		wk::Rect SlicedItem::RoundScalingGrid(const Animate::DOM::Utils::RECT& guides)
		{
			return wk::Rect(
				(int)std::ceil(std::min(guides.topLeft.x, guides.bottomRight.x)), (int)std::floor(std::max(guides.topLeft.y, guides.bottomRight.y)),
				(int)std::ceil(std::max(guides.topLeft.x, guides.bottomRight.x)), (int)std::floor(std::min(guides.topLeft.y, guides.bottomRight.y))
			);
		}
	}
}