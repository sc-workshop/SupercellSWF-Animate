#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledElement/LineSegment.h"

namespace sc
{
	namespace Adobe
	{
		FilledElementPathLineSegment::FilledElementPathLineSegment(const DOM::Utils::SEGMENT& segment) : begin(segment.line.endPoint1), end(segment.line.endPoint2)
		{
		}

		Point2D FilledElementPathLineSegment::Rasterize(float t_step) const {
			if (t_step >= 1.0f)
			{
				return Point2D(end.x, end.y);
			}
			else
			{
				return Point2D(begin.x, begin.y);
			}
		};

		float FilledElementPathLineSegment::IterationStep() const {
			return 1.0f;
		};
	}
}