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

namespace sc {
	namespace Adobe {
		class SymbolContext;

		class Point2D {
		public:
			Point2D() {};
			Point2D(float x, float y) : x(x), y(y) {};

		public:
			bool operator==(const Point2D& other) const;

			bool operator!=(const Point2D& other) const {
				return !(*this == other);
			}

		public:
			float x = 0;
			float y = 0;
		};

		class FilledElementPath {
		public:
			FilledElementPath();
			FilledElementPath(FCM::AutoPtr<DOM::Service::Shape::IPath> path);

		public:
			void fromPath(FCM::AutoPtr<DOM::Service::Shape::IPath> path);

		public:
			bool operator==(const FilledElementPath& other) const;

			bool operator!=(const FilledElementPath& other) const {
				return !(*this == other);
			}

		public:
			bool IsComplex() const { return m_is_complex; }

		public:
			std::vector<Point2D> points;

		private:
			bool m_is_complex = false;
		};

		class FilledElementRegion {
		public:
			enum class ShapeType {
				SolidColor,
				Bitmap,
				GradientColor,
			};

		public:
			FilledElementRegion(SymbolContext& symbol, FCM::AutoPtr<DOM::Service::Shape::IFilledRegion> region);

		public:
			bool operator==(const FilledElementRegion& other) const;

			bool operator!=(const FilledElementRegion& other) const {
				return !(*this == other);
			}

			// Returns true if region contour or holes used curves to determine whether it should be rasterized or not
			bool IsComplex() const;

			// Bound of contour
			DOM::Utils::RECT Bound() const;
		public:
			ShapeType type;

			DOM::Utils::COLOR solidColor{ 0 };

			FilledElementPath contour;
			std::vector<FilledElementPath> holes;
		};

		class FilledElement {
		public:
			FilledElement(SymbolContext& symbol, FCM::AutoPtr<DOM::FrameElement::IShape> shape);

		public:
			bool operator==(const FilledElement& other) const;

			bool operator!=(const FilledElement& other) const {
				return !(*this == other);
			}

		public:
			DOM::Utils::RECT Bound() const;

		public:
			std::vector<FilledElementRegion> fill;
			std::vector<FilledElementRegion> stroke;

			DOM::Utils::MATRIX2D transormation = { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };

		private:
			void AddRegions(SymbolContext& symbol, FCM::FCMListPtr regions, std::vector<FilledElementRegion>& elements);
		};
	}
}
