#include "ResourcePublisher/Writer/GraphicItem/SpriteItem.h"

namespace sc
{
	namespace Adobe
	{
		Matrix2x3<float> SpriteItem::init_matrix(const DOM::Utils::MATRIX2D& matrix)
		{
			Matrix2x3<float> result;
			result.a = matrix.a;
			result.b = matrix.b;
			result.c = matrix.c;
			result.d = matrix.d;
			result.tx = matrix.tx;
			result.ty = matrix.ty;

			return result;
		}

		SpriteItem::SpriteItem(Ref<cv::Mat> image, const DOM::Utils::MATRIX2D& matrix)
			: m_image(image), m_matrix(SpriteItem::init_matrix(matrix))
		{};

		SpriteItem SpriteItem::GetSprite()
		{
			return *this;
		}
	}
}