#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledElement/LineSegment.h"

namespace sc
{
	namespace Adobe
	{
		FilledElementPathLineSegment::FilledElementPathLineSegment(const DOM::Utils::SEGMENT& segment) :
			begin(segment.line.endPoint1.x, segment.line.endPoint1.y),
			end(segment.line.endPoint2.x, segment.line.endPoint2.y)
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

		void FilledElementPathLineSegment::Transform(const DOM::Utils::MATRIX2D& matrix)
		{
			begin.Transform(matrix);
			end.Transform(matrix);
		}

		float FilledElementPathLineSegment::IterationStep() const {
			return 1.0f;
		};
	}
}