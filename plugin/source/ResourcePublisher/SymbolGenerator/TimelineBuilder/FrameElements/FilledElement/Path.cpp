#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledElement/Path.h"

#include "Module/SessionConfig.h"

namespace sc
{
	namespace Adobe
	{
		FilledElementPath::FilledElementPath() {};
		FilledElementPath::FilledElementPath(FCM::AutoPtr<DOM::Service::Shape::IPath> path)
		{
			PluginSessionConfig& config = PluginSessionConfig::Instance();
			FCM::FCMListPtr edges;
			uint32_t edgesCount = 0;
			path->GetEdges(edges.m_Ptr);
			edges->Count(edgesCount);

			for (uint32_t i = 0; edgesCount > i; i++) {
				FCM::AutoPtr<DOM::Service::Shape::IEdge> edge = edges[i];

				DOM::Utils::SEGMENT segment;
				segment.structSize = sizeof(segment);
				edge->GetSegment(segment);

				switch (segment.segmentType)
				{
				case DOM::Utils::SegmentType::LINE_SEGMENT:
					CreateSegment<FilledElementPathLineSegment>(segment);
					break;

				case DOM::Utils::SegmentType::CUBIC_BEZIER_SEGMENT:
					CreateSegment<FilledElementPathCubicSegment>(segment);
					break;

				case DOM::Utils::SegmentType::QUAD_BEZIER_SEGMENT:
					CreateSegment<FilledElementPathQuadSegment>(segment);
					break;
				default:
					break;
				}
			}
		};

		const FilledElementPathSegment& FilledElementPath::GetSegment(size_t index) const
		{
			return *m_segments[index];
		}

		FilledElementPathSegment& FilledElementPath::GetSegment(size_t index)
		{
			return *m_segments[index];
		}

		void FilledElementPath::Rasterize(std::vector<Point2D>& points) const
		{
			points.clear();

			for (size_t i = 0; Count() > i; i++)
			{
				const FilledElementPathSegment& segment = GetSegment(i);

				for (Point2D& point : segment)
				{
					if (!points.empty())
					{
						Point2D& last_point = points[points.size() - 1];
						if (point.x == last_point.x && point.y == last_point.y)
						{
							continue;
						}
					}

					points.emplace_back(point.x, point.y);
				}
			}
		}

		size_t FilledElementPath::Count() const
		{
			return m_segments.size();
		}

		void FilledElementPath::Transform(const DOM::Utils::MATRIX2D& matrix)
		{
			for (size_t i = 0; Count() > i; i++)
			{
				FilledElementPathSegment& segment = GetSegment(i);
				segment.Transform(matrix);
			}
		}

		bool FilledElementPath::operator==(const FilledElementPath& other) const {
			if (Count() != other.Count()) { return false; }

			for (uint32_t i = 0; m_segments.size() > i; i++) {
				if (m_segments[i] != other.m_segments[i]) {
					return false;
				}
			}

			return true;
		}
	}
}