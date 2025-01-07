#include "SpriteItem.h"

namespace sc
{
	namespace Adobe
	{
		BitmapItem::BitmapItem(
			Animate::Publisher::SymbolContext& context,
			wk::RawImageRef image,
			const Animate::DOM::Utils::MATRIX2D& matrix
		) : GraphicItem(context), m_image(image)
		{
			m_matrix = matrix;
		};

		BitmapItem::BitmapItem(Animate::Publisher::SymbolContext& context) : GraphicItem(context)
		{

		}
	}
}