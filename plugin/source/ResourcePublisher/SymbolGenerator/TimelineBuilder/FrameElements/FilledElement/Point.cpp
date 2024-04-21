#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledElement/Point.h"

namespace sc
{
	namespace Adobe
	{
		bool Point2D::operator==(const Point2D& other) const {
			return x == other.x && y == other.y;
		}

		void Point2D::Transform(const DOM::Utils::MATRIX2D& matrix)
		{
			float point_x = x;
			float point_y = y;

			x = (matrix.a * point_x) + (matrix.c * point_y) + matrix.tx;
			y = (matrix.b * point_x) + (matrix.d * point_y) + matrix.ty;
		}
	}
}