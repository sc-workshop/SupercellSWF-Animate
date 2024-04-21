#pragma once

#include "AnimateSDK/core/common/FCMTypes.h"
#include "AnimateSDK/app/DOM/Service/Shape/IFilledRegion.h"
#include "AnimateSDK/app/DOM/FillStyle/ISolidFillStyle.h"
#include "AnimateSDK/app/DOM/FillStyle/IBitmapFillStyle.h"
#include "AnimateSDK/app/DOM/FillStyle/IGradientFillStyle.h"

#include "Path.h"

#include "Module/PluginException.h"

namespace sc
{
	namespace Adobe
	{
		class SymbolContext;

		class FilledElementRegion {
		public:
			struct SolidFill
			{
				DOM::Utils::COLOR color;
			};

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

			// Bound of contour
			DOM::Utils::RECT Bound() const;

			void Transform(const DOM::Utils::MATRIX2D& matrix);

		public:
			ShapeType type;

			union
			{
				SolidFill solid;
			};

			FilledElementPath contour;
			std::vector<FilledElementPath> holes;
		};
	}
}