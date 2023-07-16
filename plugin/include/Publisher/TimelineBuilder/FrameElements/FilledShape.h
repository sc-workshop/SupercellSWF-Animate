#pragma once

#include "FCMPluginInterface.h"
#include "ApplicationFCMPublicIDs.h"
#include "Module/AppContext.h"

#include "DOM/FrameElement/IShape.h"
#include "DOM/Service/Shape/IFilledRegion.h"

// Fills polygon
#include "DOM/Service/Shape/IRegionGeneratorService.h"
#include "DOM/Service/Shape/IPath.h"
#include "DOM/Service/Shape/IEdge.h"

// Fill style
#include "DOM/FillStyle/ISolidFillStyle.h"

// Stroke
#include "DOM/Service/Shape/IShapeService.h"

#include "DOM/Utils/DOMTypes.h"

using namespace DOM::Service::Shape;

namespace sc {
	namespace Adobe {
		struct Point2D {
			bool operator==(const Point2D& other) const;

			bool operator!=(const Point2D& other) const {
				return !(*this == other);
			}

			float x;
			float y;
		};

		struct FilledShapePath {
			FilledShapePath(FCM::AutoPtr<IPath> path);

			bool operator==(const FilledShapePath& other) const;

			bool operator!=(const FilledShapePath& other) const {
				return !(*this == other);
			}

			std::vector<Point2D> points;
		};

		enum class FilledShapeType {
			SolidColor,
			Bitmap,
			GradientColor
		};

		struct FilledShapeRegion {
			FilledShapeRegion(FCM::AutoPtr<IFilledRegion> region);

			bool operator==(const FilledShapeRegion& other) const;

			bool operator!=(const FilledShapeRegion& other) const {
				return !(*this == other);
			}

			FilledShapeType type;

			uint32_t solidColor = 0;

			std::shared_ptr<FilledShapePath> contour;
			std::vector<std::shared_ptr<FilledShapePath>> holes;
		};

		struct FilledShape {
			FilledShape(AppContext& context, FCM::AutoPtr<DOM::FrameElement::IShape> shape);

			bool operator==(const FilledShape& other) const;

			bool operator!=(const FilledShape& other) const {
				return !(*this == other);
			}

			std::vector<FilledShapeRegion> fill;
			std::vector<FilledShapeRegion> stroke;
		};
	}
}