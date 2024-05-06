#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledElement/Segment.h"

#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledElement/LineSegment.h"
#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledElement/CubicSegment.h"
#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledElement/QuadSegment.h"

namespace sc
{
	namespace Adobe
	{
		bool FilledElementPathSegment::operator==(const FilledElementPathSegment& other) const
		{
			if (SegmentType() != other.SegmentType()) return false;

			switch (SegmentType())
			{
			case Type::Line:
			{
				FilledElementPathLineSegment& current_segment = *(FilledElementPathLineSegment*)this;
				FilledElementPathLineSegment& other_segment = *(FilledElementPathLineSegment*)&other;

				if (current_segment.begin != other_segment.begin) return false;
				if (current_segment.end != other_segment.end) return false;

				return true;
			}
			case Type::Cubic:
			{
				FilledElementPathCubicSegment& current_segment = *(FilledElementPathCubicSegment*)this;
				FilledElementPathCubicSegment& other_segment = *(FilledElementPathCubicSegment*)&other;

				if (current_segment.begin != other_segment.begin) return false;
				if (current_segment.end != other_segment.end) return false;
				if (current_segment.control_l != other_segment.control_l) return false;
				if (current_segment.control_r != other_segment.control_r) return false;

				return true;
			}
			case Type::Quad:
			{
				FilledElementPathQuadSegment& current_segment = *(FilledElementPathQuadSegment*)this;
				FilledElementPathQuadSegment& other_segment = *(FilledElementPathQuadSegment*)&other;

				if (current_segment.begin != other_segment.begin) return false;
				if (current_segment.end != other_segment.end) return false;
				if (current_segment.control != other_segment.control) return false;

				return true;
			}
			default:
				break;
			}
		};
	}
}