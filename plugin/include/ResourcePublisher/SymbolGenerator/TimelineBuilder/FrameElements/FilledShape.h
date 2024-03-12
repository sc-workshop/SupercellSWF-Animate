#pragma once

#include <vector>
#include <memory>

#include "AnimateSDK/core/common/FCMPluginInterface.h"
#include "AnimateSDK/app/ApplicationFCMPublicIDs.h"

#include "AnimateSDK/app/DOM/FrameElement/IShape.h"
#include "AnimateSDK/app/DOM/Service/Shape/IFilledRegion.h"

// Fills polygon
#include "AnimateSDK/app/DOM/Service/Shape/IRegionGeneratorService.h"
#include "AnimateSDK/app/DOM/Service/Shape/IPath.h"
#include "AnimateSDK/app/DOM/Service/Shape/IEdge.h"

// Fill style
#include "AnimateSDK/app/DOM/FillStyle/ISolidFillStyle.h"

// Stroke
#include "AnimateSDK/app/DOM/Service/Shape/IShapeService.h"

#include "AnimateSDK/app/DOM/Utils/DOMTypes.h"

#include "Module/SymbolContext.h"

namespace sc {
	namespace Adobe {
		struct Point2D {
			Point2D() {};
			Point2D(float x, float y) : x(x), y(y) {};

			bool operator==(const Point2D& other) const;

			bool operator!=(const Point2D& other) const {
				return !(*this == other);
			}

			float x = 0;
			float y = 0;
		};

		struct FilledShapePath {
			FilledShapePath();
			FilledShapePath(FCM::AutoPtr<DOM::Service::Shape::IPath> path);

			void fromPath(FCM::AutoPtr<DOM::Service::Shape::IPath> path);

			bool operator==(const FilledShapePath& other) const;

			bool operator!=(const FilledShapePath& other) const {
				return !(*this == other);
			}

			std::vector<Point2D> points;
		};

		struct FilledShapeRegion {
		public:
			enum class ShapeType {
				SolidColor,
				Bitmap,
				GradientColor,
			};

		public:
			FilledShapeRegion(SymbolContext& symbol, FCM::AutoPtr<DOM::Service::Shape::IFilledRegion> region);

			bool operator==(const FilledShapeRegion& other) const;

			bool operator!=(const FilledShapeRegion& other) const {
				return !(*this == other);
			}

			ShapeType type;

			DOM::Utils::COLOR solidColor{ 0 };

			FilledShapePath contour;
			std::vector<FilledShapePath> holes;
		};

		struct FilledShape {
			FilledShape(SymbolContext& symbol, FCM::AutoPtr<DOM::FrameElement::IShape> shape);

			bool operator==(const FilledShape& other) const;

			bool operator!=(const FilledShape& other) const {
				return !(*this == other);
			}

			std::vector<FilledShapeRegion> fill;
			std::vector<FilledShapeRegion> stroke;
		};
	}
}
