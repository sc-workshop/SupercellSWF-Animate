#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledElement/QuadSegment.h"

namespace sc
{
	namespace Adobe
	{
		FilledElementPathQuadSegment::FilledElementPathQuadSegment(const DOM::Utils::SEGMENT& segment) :
			begin(segment.quadBezierCurve.anchor1),
			control(segment.quadBezierCurve.control),
			end(segment.quadBezierCurve.anchor2)
		{
		}

		Point2D FilledElementPathQuadSegment::Rasterize(float t_step) const {
			float mt = 1.0f - t_step;
			float x = mt * mt * begin.x + 2 * mt * t_step * control.x + t_step * t_step * end.x;
			float y = mt * mt * begin.y + 2 * mt * t_step * control.y + t_step * t_step * end.y;

			return { x, y };
		};

		float FilledElementPathQuadSegment::IterationStep() const {
			Point2D distance(0, 0);
			distance.x += std::abs(begin.x - control.x);
			distance.y += std::abs(begin.y - control.y);

			distance.x += std::abs(control.x - end.x);
			distance.y += std::abs(control.y - end.y);

			return (1.0f / distance.x) + (1.0f / distance.y);
		};
	}
}