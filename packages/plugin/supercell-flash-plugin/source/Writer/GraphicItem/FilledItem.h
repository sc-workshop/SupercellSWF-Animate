#pragma once

#include "AnimatePublisher.h"
#include "GraphicItem.h"

namespace sc
{
	namespace Adobe
	{
		class FilledItemContour
		{
		public:
			using Point = Animate::Publisher::Point2D;

		public:
			FilledItemContour(const std::vector<Point>& contour) : m_contour(contour)
			{
			}

			const std::vector<Point>& Contour() const
			{
				return m_contour;
			}

		public:
			std::vector<Point> m_contour;
		};

		// Class that represents simple filled contour
		class FilledItem : public GraphicItem
		{
		public:
			FilledItem(
				Animate::Publisher::SymbolContext& context,
				const std::vector<FilledItemContour>& contour,
				const Animate::DOM::Utils::COLOR& color,
				const Animate::DOM::Utils::MATRIX2D& matrix = {}
			) :
				GraphicItem(context),
				contours(contour),
				m_color(
					color.red,
					color.green,
					color.blue,
					color.alpha
				)
			{
				m_matrix = matrix;
			};

			virtual ~FilledItem() = default;

		public:
			virtual bool IsSolidColor() const
			{
				return true;
			}

		public:
			virtual const wk::ColorRGBA& Color() const
			{
				return m_color;
			}

			const std::vector<FilledItemContour> contours;

		private:
			wk::ColorRGBA m_color;
		};
	}
}