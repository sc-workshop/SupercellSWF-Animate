#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledElement.h"

#include "Module/PluginException.h"
#include "Module/SessionConfig.h"
#include "Module/PluginContext.h"
#include "Module/Symbol/SymbolContext.h"

#define STEP_COUNT 14

namespace sc {
	namespace Adobe {
		namespace Curve {
			// De Casteljau's algorithm to rasterize a Bezier curve
			void rasterizeQuadBezier(std::vector<Point2D>& points, const Point2D& p0, const Point2D& p1, const Point2D& p2, float tStep) {
				for (float t = 0.0f; t <= 1.0f; t += tStep) {
					float mt = 1.0f - t;
					float x = mt * mt * p0.x + 2 * mt * t * p1.x + t * t * p2.x;
					float y = mt * mt * p0.y + 2 * mt * t * p1.y + t * t * p2.y;
					points.emplace_back(x, y);
				}
			}

			void rasterizeCubicBezier(std::vector<Point2D>& points, const Point2D& p0, const Point2D& p1, const Point2D& p2, const Point2D& p3, float tStep) {
				for (float t = 0.0f; t <= 1.0f; t += tStep) {
					float mt = 1.0f - t;
					float mt2 = mt * mt;
					float t2 = t * t;

					float x = mt2 * mt * p0.x + 3 * mt2 * t * p1.x + 3 * mt * t2 * p2.x + t2 * t * p3.x;
					float y = mt2 * mt * p0.y + 3 * mt2 * t * p1.y + 3 * mt * t2 * p2.y + t2 * t * p3.y;

					points.emplace_back(x, y);
				}
			}
		}
	}
}

namespace sc {
	namespace Adobe {
		bool Point2D::operator==(const Point2D& other) const {
			return x == other.x && y == other.y;
		}

#pragma region
		FilledElementPath::FilledElementPath() {};
		FilledElementPath::FilledElementPath(FCM::AutoPtr<DOM::Service::Shape::IPath> path)
		{
			fromPath(path);
		};

		void FilledElementPath::fromPath(FCM::AutoPtr<DOM::Service::Shape::IPath> path) {
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
				{
					Point2D begin = *reinterpret_cast<Point2D*>(&segment.line.endPoint1);
					Point2D end = *reinterpret_cast<Point2D*>(&segment.line.endPoint2);

					bool validPoint = false;
					for (uint32_t p = 0; points.size() > p; p++) {
						if (points[p] == begin) {
							points.insert(points.begin() + p, end);
							validPoint = true;
							break;
						}
					}

					if (!validPoint) {
						points.push_back(begin);
						points.push_back(end);
					}
				}
				break;

				case DOM::Utils::SegmentType::CUBIC_BEZIER_SEGMENT:
				case DOM::Utils::SegmentType::QUAD_BEZIER_SEGMENT:
					if (config.filledShapeOptimization) {
						Point2D begin = *reinterpret_cast<Point2D*>(&segment.quadBezierCurve.anchor1);
						Point2D end = *reinterpret_cast<Point2D*>(&segment.quadBezierCurve.anchor2);

						bool validPoint = false;
						for (uint32_t p = 0; points.size() > p; p++) {
							if (points[p] == begin) {
								points.insert(points.begin() + p, end);
								validPoint = true;
								break;
							}
						}

						if (!validPoint) {
							points.push_back(begin);
							points.push_back(end);
						}
					}
					else {
						if (segment.segmentType == DOM::Utils::SegmentType::QUAD_BEZIER_SEGMENT) {
							Point2D anchor1 = { segment.quadBezierCurve.anchor1.x, segment.quadBezierCurve.anchor1.y };
							Point2D control = { segment.quadBezierCurve.control.x, segment.quadBezierCurve.control.y };
							Point2D anchor2 = { segment.quadBezierCurve.anchor2.x, segment.quadBezierCurve.anchor2.y };

							Point2D distance = { std::abs(anchor2.x - anchor1.x), std::abs(anchor2.y - anchor1.y) };

							float t = (5 / distance.x) + (5 / distance.y);

							Curve::rasterizeQuadBezier(
								points,
								anchor1,
								control,
								anchor2,
								t
							);
						}
						else if (segment.segmentType == DOM::Utils::SegmentType::CUBIC_BEZIER_SEGMENT) {
							Point2D anchor1 = { segment.cubicBezierCurve.anchor1.x , segment.cubicBezierCurve.anchor1.y };
							Point2D anchor2 = { segment.cubicBezierCurve.anchor2.x , segment.cubicBezierCurve.anchor2.y };
							Point2D control1 = { segment.cubicBezierCurve.control1.x , segment.cubicBezierCurve.control1.y };
							Point2D control2 = { segment.cubicBezierCurve.control2.x , segment.cubicBezierCurve.control2.y };

							Point2D distance = { std::abs(anchor2.x - anchor1.x), std::abs(anchor2.y - anchor1.y) };

							float t = (5 / distance.x) + (5 / distance.y);

							Curve::rasterizeCubicBezier(
								points,
								anchor1,
								anchor2,
								control1,
								control2,
								t
							);
						}
					}
					break;

				default:
					break;
				}
			}
		}

		bool FilledElementPath::operator==(const FilledElementPath& other) const {
			if (points.size() != other.points.size()) { return false; }

			for (uint32_t i = 0; points.size() > i; i++) {
				if (points[i] != other.points[i]) {
					return false;
				}
			}

			return true;
		}
#pragma endregion Path

#pragma region
		FilledElementRegion::FilledElementRegion(SymbolContext& symbol, FCM::AutoPtr<DOM::Service::Shape::IFilledRegion> region) {
			// Fill style
			{
				FCM::AutoPtr<FCM::IFCMUnknown> unknown_style;
				region->GetFillStyle(unknown_style.m_Ptr);

				FCM::AutoPtr<DOM::FillStyle::ISolidFillStyle> solid = unknown_style;

				if (solid) {
					type = ShapeType::SolidColor;
					solid->GetColor(solidColor);
				}
				else {
					throw PluginException(
						"TID_UNKNOWN_FILL_STYLE_TYPE",
						symbol.name.c_str()
					);
				}
			}

			// Contour
			{
				FCM::AutoPtr<DOM::Service::Shape::IPath> polygonPath;
				region->GetBoundary(polygonPath.m_Ptr);
				contour.fromPath(polygonPath);
			}

			// Holes
			{
				FCM::FCMListPtr holePaths;
				uint32_t holePathsCount = 0;
				region->GetHoles(holePaths.m_Ptr);
				holePaths->Count(holePathsCount);

				for (uint32_t i = 0; holePathsCount > i; i++) {
					FCM::AutoPtr<DOM::Service::Shape::IPath> holePath = holePaths[i];

					holes.emplace_back(holePath);
				}
			}
		}

		bool FilledElementRegion::operator==(const FilledElementRegion& other) const {
			if (type != other.type) { return false; }
			if (contour != other.contour || holes.size() != other.holes.size()) { return false; }

			switch (type)
			{
			case ShapeType::SolidColor:
				if (*(uint32_t*)&solidColor != *(uint32_t*)&other.solidColor) { return false; };
				break;
			default:
				return false;
			}

			for (uint32_t i = 0; holes.size() > i; i++) {
				if (holes[i] != other.holes[i]) {
					return false;
				}
			}

			return true;
		}

#pragma endregion Region

#pragma region
		FilledElement::FilledElement(SymbolContext& symbol, FCM::AutoPtr<DOM::FrameElement::IShape> shape) {
			PluginContext& context = PluginContext::Instance();

			auto stroke_generator =
				context.getService<DOM::Service::Shape::IShapeService>(DOM::FLA_SHAPE_SERVICE);

			auto region_generator =
				context.getService<DOM::Service::Shape::IRegionGeneratorService>(DOM::FLA_REGION_GENERATOR_SERVICE);

			{
				FCM::FCMListPtr regions;
				region_generator->GetFilledRegions(shape, regions.m_Ptr);

				AddRegions(symbol, regions, fill);
			}

			{
				FCM::AutoPtr<DOM::FrameElement::IShape> stroke_fill;
				stroke_generator->ConvertStrokeToFill(shape, stroke_fill.m_Ptr);

				FCM::FCMListPtr regions;
				region_generator->GetFilledRegions(stroke_fill, regions.m_Ptr);

				AddRegions(symbol, regions, stroke);
			}
		}

		bool FilledElement::operator==(const FilledElement& other) const {
			if (fill.size() != other.fill.size() || stroke.size() != other.stroke.size()) { return false; }

			for (uint32_t i = 0; fill.size() > i; i++) {
				if (fill[i] != other.fill[i]) {
					return false;
				}
			}

			for (uint32_t i = 0; stroke.size() > i; i++) {
				if (stroke[i] != other.stroke[i]) {
					return false;
				}
			}

			return true;
		}

		void FilledElement::AddRegions(SymbolContext& symbol, FCM::FCMListPtr regions, std::vector<FilledElementRegion>& elements)
		{
			uint32_t region_count;
			regions->Count(region_count);

			for (uint32_t i = 0; region_count > i; i++) {
				FCM::AutoPtr<DOM::Service::Shape::IFilledRegion> region = regions[i];
				if (!region) continue;

				elements.emplace_back(symbol, region);
			}
		}
	}
#pragma endregion Element
}