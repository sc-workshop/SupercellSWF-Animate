#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledElement/CubicSegment.h"

namespace sc
{
	namespace Adobe
	{
		FilledElementPathCubicSegment::FilledElementPathCubicSegment(const DOM::Utils::SEGMENT& segment) :
			begin(segment.cubicBezierCurve.anchor1.x, segment.cubicBezierCurve.anchor1.y),
			control_l(segment.cubicBezierCurve.control1.x, segment.cubicBezierCurve.control1.y),
			control_r(segment.cubicBezierCurve.control2.x, segment.cubicBezierCurve.control2.y),
			end(segment.cubicBezierCurve.anchor2.x, segment.cubicBezierCurve.anchor2.y)
		{
		}

		Point2D FilledElementPathCubicSegment::Rasterize(float t_step) const {
			float mt = 1.0f - t_step;
			float mt2 = mt * mt;
			float t2 = t_step * t_step;

			float x = mt2 * mt * begin.x + 3 * mt2 * t_step * end.x + 3 * mt * t2 * control_l.x + t2 * t_step * control_r.x;
			float y = mt2 * mt * begin.y + 3 * mt2 * t_step * end.y + 3 * mt * t2 * control_l.y + t2 * t_step * control_r.y;

			return { x, y };
		};

		float FilledElementPathCubicSegment::IterationStep() const {
			Point2D distance = { std::abs(end.x - begin.x), std::abs(end.y - begin.y) };

			return (1.0f / distance.x) + (1.0f / distance.y);
		};

		void FilledElementPathCubicSegment::Transform(const DOM::Utils::MATRIX2D& matrix)
		{
			begin.Transform(matrix);
			control_l.Transform(matrix);
			control_r.Transform(matrix);
			end.Transform(matrix);
		}
	}
}