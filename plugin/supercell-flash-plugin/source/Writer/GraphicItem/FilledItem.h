#pragma once

#include "AnimatePublisher.h"
#include "GraphicItem.h"

#include <opencv2/opencv.hpp>
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
			const std::vector<Point> m_contour;
		};

		// Class that represents simple filled contour
		class FilledItem : public GraphicItem
		{
		public:
			FilledItem(
				const std::vector<FilledItemContour>& contour,
				const Animate::DOM::Utils::COLOR& color,
				const Animate::DOM::Utils::MATRIX2D& matrix = { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f }
			) :
				contours(contour),
				m_matrix(matrix.a, matrix.b, matrix.c, matrix.d, matrix.tx, matrix.ty),
				m_color(
					color.blue,
					color.green,
					color.red,
					color.alpha
				)
			{
			};

			virtual ~FilledItem() = default;

		public:
			virtual bool IsSolidColor() const
			{
				return true;
			}

		public:
			virtual const cv::Scalar& Color() const
			{
				return m_color;
			}

			virtual wk::Matrix2D Transformation() const
			{
				return m_matrix;
			}

			const std::vector<FilledItemContour> contours;
		private:
			const wk::Matrix2D m_matrix;
			cv::Scalar m_color;
		};
	}
}