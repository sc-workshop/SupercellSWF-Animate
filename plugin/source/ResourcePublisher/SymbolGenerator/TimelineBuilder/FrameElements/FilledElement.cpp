#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledElement.h"

#include "Module/PluginException.h"
#include "Module/SessionConfig.h"
#include "Module/PluginContext.h"
#include "Module/Symbol/SymbolContext.h"

// namespace sc {
// 	namespace Adobe {
// 		namespace Curve {
// 			// De Casteljau's algorithm to rasterize a Bezier curve
// 			void rasterizeQuadBezier(std::vector<Point2D>& points, const Point2D& p0, const Point2D& p1, const Point2D& p2, float tStep) {
// 				for (float t = 0.0f; t <= 1.0f; t += tStep) {
// 					float mt = 1.0f - t;
// 					float x = mt * mt * p0.x + 2 * mt * t * p1.x + t * t * p2.x;
// 					float y = mt * mt * p0.y + 2 * mt * t * p1.y + t * t * p2.y;
// 					points.emplace_back(x, y);
// 				}
// 			}
//
// 			void rasterizeCubicBezier(std::vector<Point2D>& points, const Point2D& p0, const Point2D& p1, const Point2D& p2, const Point2D& p3, float tStep) {
// 				for (float t = 0.0f; t <= 1.0f; t += tStep) {
// 					float mt = 1.0f - t;
// 					float mt2 = mt * mt;
// 					float t2 = t * t;
//
// 					float x = mt2 * mt * p0.x + 3 * mt2 * t * p1.x + 3 * mt * t2 * p2.x + t2 * t * p3.x;
// 					float y = mt2 * mt * p0.y + 3 * mt2 * t * p1.y + 3 * mt * t2 * p2.y + t2 * t * p3.y;
//
// 					points.emplace_back(x, y);
// 				}
// 			}
// 		}
// 	}
// }

namespace sc {
	namespace Adobe {
		FilledElement::FilledElement(SymbolContext& symbol, FCM::AutoPtr<DOM::FrameElement::IShape> shape) {
			PluginContext& context = PluginContext::Instance();

			auto stroke_generator =
				context.GetService<DOM::Service::Shape::IShapeService>(DOM::FLA_SHAPE_SERVICE);

			auto region_generator =
				context.GetService<DOM::Service::Shape::IRegionGeneratorService>(DOM::FLA_REGION_GENERATOR_SERVICE);

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

		DOM::Utils::RECT FilledElement::Bound() const
		{
			DOM::Utils::RECT result{
				{std::numeric_limits<float>::min(),
				std::numeric_limits<float>::min()},
				{std::numeric_limits<float>::max(),
				std::numeric_limits<float>::max()}
			};

			for (const FilledElementRegion& region : fill)
			{
				const DOM::Utils::RECT region_bound = region.Bound();

				result.topLeft.x = std::max(region_bound.topLeft.x, result.topLeft.x);
				result.topLeft.y = std::max(region_bound.topLeft.y, result.topLeft.y);
				result.bottomRight.x = std::min(region_bound.bottomRight.x, result.bottomRight.x);
				result.bottomRight.y = std::min(region_bound.bottomRight.y, result.bottomRight.y);
			}

			for (const FilledElementRegion& region : stroke)
			{
				const DOM::Utils::RECT region_bound = region.Bound();

				result.topLeft.x = std::max(region_bound.topLeft.x, result.topLeft.x);
				result.topLeft.y = std::max(region_bound.topLeft.y, result.topLeft.y);
				result.bottomRight.x = std::min(region_bound.bottomRight.x, result.bottomRight.x);
				result.bottomRight.y = std::min(region_bound.bottomRight.y, result.bottomRight.y);
			}

			return result;
		}
	}
}