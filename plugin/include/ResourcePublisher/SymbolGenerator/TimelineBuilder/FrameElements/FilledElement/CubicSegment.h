#pragma once

#include "AnimateSDK/app/DOM/Utils/DOMTypes.h"
#include "Segment.h"

namespace sc
{
	namespace Adobe
	{
		class FilledElementPathCubicSegment : public FilledElementPathSegment
		{
		public:
			FilledElementPathCubicSegment(const DOM::Utils::SEGMENT& segment);

		public:
			virtual FilledElementPathSegment::Type SegmentType() const {
				return FilledElementPathSegment::Type::Cubic;
			};

			virtual Point2D Rasterize(float t_step) const;

			virtual float IterationStep() const;

		public:
			DOM::Utils::POINT2D begin;
			DOM::Utils::POINT2D control_l;
			DOM::Utils::POINT2D control_r;
			DOM::Utils::POINT2D end;
		};
	}
}