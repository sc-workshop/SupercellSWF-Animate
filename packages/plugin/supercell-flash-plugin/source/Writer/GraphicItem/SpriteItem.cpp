#include "SpriteItem.h"

namespace sc
{
	namespace Adobe
	{
		BitmapItem::BitmapItem(
			Animate::Publisher::SymbolContext& context,
			wk::RawImageRef image,
			const Animate::DOM::Utils::MATRIX2D& matrix,
			bool rasterized
		) : GraphicItem(context), m_image(image), m_is_rasterized(rasterized)
		{
			m_matrix = matrix;
		};

		BitmapItem::BitmapItem(Animate::Publisher::SymbolContext& context) : GraphicItem(context)
		{

		}
	}
}