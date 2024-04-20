#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledElement/Point.h"

namespace sc
{
	namespace Adobe
	{
		bool Point2D::operator==(const Point2D& other) const {
			return x == other.x && y == other.y;
		}
	}
}