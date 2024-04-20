#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledElement/Region.h"

#include "Module/Symbol/SymbolContext.h"

namespace sc
{
	namespace Adobe
	{
		FilledElementRegion::FilledElementRegion(SymbolContext& symbol, FCM::AutoPtr<DOM::Service::Shape::IFilledRegion> region) {
			// Fill style
			{
				FCM::AutoPtr<FCM::IFCMUnknown> unknown_style;
				region->GetFillStyle(unknown_style.m_Ptr);

				FCM::AutoPtr<DOM::FillStyle::ISolidFillStyle> solid_style = unknown_style;

				if (solid_style) {
					type = ShapeType::SolidColor;
					solid_style->GetColor(solid.color);
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
				contour = FilledElementPath(polygonPath);
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
				if (*(uint32_t*)&solid.color != *(uint32_t*)&other.solid.color) { return false; };
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

		DOM::Utils::RECT FilledElementRegion::Bound() const
		{
			DOM::Utils::RECT result{
				{std::numeric_limits<float>::min(),
				std::numeric_limits<float>::min()},
				{std::numeric_limits<float>::max(),
				std::numeric_limits<float>::max()}
			};

			for (size_t i = 0; contour.Count() > i; i++)
			{
				const FilledElementPathSegment& path_point = contour.GetSegment(i);
				for (Point2D& point : path_point)
				{
					result.bottomRight.x = std::min(result.bottomRight.x, point.x);
					result.bottomRight.y = std::min(result.bottomRight.y, point.y);

					result.topLeft.x = std::max(result.topLeft.x, point.x);
					result.topLeft.y = std::max(result.topLeft.y, point.y);
				}
			}

			return result;
		}
	}
}