#pragma once

#include "Publisher/TimelineBuilder/FrameElements/FilledShape.h"

namespace sc {
	namespace Adobe {
		bool Point2D::operator==(const Point2D& other) const {
			return x == other.x && y == other.y;
		}


		FilledShapePath::FilledShapePath(FCM::AutoPtr<IPath> path) {
			FCM::FCMListPtr edges;
			uint32_t edgesCount = 0;
			path->GetEdges(edges.m_Ptr);
			edges->Count(edgesCount);

			for (uint32_t i = 0; edgesCount > i; i++) {
				FCM::AutoPtr<IEdge> edge = edges[i];

				if (!edge) throw std::exception("Failed to get FilledShape edge");

				DOM::Utils::SEGMENT segment;
				segment.structSize = sizeof(segment);
				edge->GetSegment(segment);

				Point2D segmentPoint;

				switch (segment.segmentType)
				{
				case DOM::Utils::SegmentType::LINE_SEGMENT:
					if (i == 0) {
						segmentPoint = *reinterpret_cast<Point2D*>(&segment.line.endPoint1);
					}
					else {
						segmentPoint = *reinterpret_cast<Point2D*>(&segment.line.endPoint2);
					}
				case DOM::Utils::SegmentType::CUBIC_BEZIER_SEGMENT:
				case DOM::Utils::SegmentType::QUAD_BEZIER_SEGMENT:
					segmentPoint = *reinterpret_cast<Point2D*>(&segment.quadBezierCurve.control);
				default:
					break;
				}

				points.push_back(segmentPoint);
			}
		}

		bool FilledShapePath::operator==(const FilledShapePath& other) const {
			if (points.size() != other.points.size()) { return false; }

			for (uint32_t i = 0; points.size() > i; i++) {
				if (points[i] != other.points[i]) {
					return false;
				}
			}

			return true;
		}


		FilledShapeRegion::FilledShapeRegion(FCM::AutoPtr<IFilledRegion> region) {
			// Contour
			{
				FCM::AutoPtr<IPath> polygonPath;
				region->GetBoundary(polygonPath.m_Ptr);
				contour = std::shared_ptr<FilledShapePath>(
					new FilledShapePath(polygonPath)
					);
			}

			// Holes
			{
				FCM::FCMListPtr holePaths;
				uint32_t holePathsCount = 0;
				region->GetHoles(holePaths.m_Ptr);
				holePaths->Count(holePathsCount);

				for (uint32_t i = 0; holePathsCount > i; i++) {
					FCM::AutoPtr<IPath> holePath = holePaths[i];

					holes.push_back(
						std::shared_ptr<FilledShapePath>(
							new FilledShapePath(holePath)
							)
					);
				}

			}

			// Fill style
			{

				FCM::AutoPtr<FCM::IFCMUnknown> unknownFillStyle;
				region->GetFillStyle(unknownFillStyle.m_Ptr);

				FCM::AutoPtr<DOM::FillStyle::ISolidFillStyle> solidStyle = unknownFillStyle;

				if (solidStyle) {
					type = FilledShapeType::SolidColor;
					solidStyle->GetColor(
						*reinterpret_cast<DOM::Utils::COLOR*>(&solidColor)
					);
				}
				else {
					throw std::exception("Unknown fill style in FilledShape");
				}
			}
		}

		bool FilledShapeRegion::operator==(const FilledShapeRegion& other) const {
			if (type != other.type) { return false; }
			if (*contour != *other.contour || holes.size() != other.holes.size()) { return false; }

			switch (type)
			{
			case sc::Adobe::FilledShapeType::SolidColor:
				if (solidColor != other.solidColor) { return false; };
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

		FilledShape::FilledShape(AppContext& context, FCM::AutoPtr<DOM::FrameElement::IShape> shape) {
			FCM::AutoPtr<IRegionGeneratorService> filledShapeGenerator =
				context.getService<IRegionGeneratorService>(DOM::FLA_REGION_GENERATOR_SERVICE);

			FCM::AutoPtr<IShapeService> strokeGenerator =
				context.getService<IShapeService>(DOM::FLA_SHAPE_SERVICE);

			{
				FCM::FCMListPtr fiilRegions;
				filledShapeGenerator->GetFilledRegions(shape, fiilRegions.m_Ptr);
				uint32_t regionsCount;
				fiilRegions->Count(regionsCount);

				for (uint32_t i = 0; regionsCount > i; i++) {
					FCM::AutoPtr<IFilledRegion> filledRegion = fiilRegions[i];
					if (!filledRegion) continue;

					fill.push_back(
						FilledShapeRegion(filledRegion)
					);
				}
			}

			{
				FCM::AutoPtr<DOM::FrameElement::IShape> strokeShape;
				strokeGenerator->ConvertStrokeToFill(shape, strokeShape.m_Ptr);

				FCM::FCMListPtr strokeRegions;
				filledShapeGenerator->GetFilledRegions(strokeShape, strokeRegions.m_Ptr);
				uint32_t regionsCount;
				strokeRegions->Count(regionsCount);

				for (uint32_t i = 0; regionsCount > i; i++) {
					FCM::AutoPtr<IFilledRegion> filledRegion = strokeRegions[i];
					if (!filledRegion) continue;

					stroke.push_back(
						FilledShapeRegion(filledRegion)
					);
				}
			}
		}

		bool FilledShape::operator==(const FilledShape& other) const {
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
	}
}