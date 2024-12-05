#include "SpriteItem.h"

namespace sc
{
	namespace Adobe
	{
		wk::Matrix2D SpriteItem::init_matrix(const Animate::DOM::Utils::MATRIX2D& matrix)
		{
			wk::Matrix2D result;
			result.a = matrix.a;
			result.b = matrix.b;
			result.c = matrix.c;
			result.d = matrix.d;
			result.tx = matrix.tx;
			result.ty = matrix.ty;

			return result;
		}

		SpriteItem::SpriteItem(wk::Ref<cv::Mat> image, const Animate::DOM::Utils::MATRIX2D& matrix)
			: m_image(image), m_matrix(SpriteItem::init_matrix(matrix))
		{};
	}
}