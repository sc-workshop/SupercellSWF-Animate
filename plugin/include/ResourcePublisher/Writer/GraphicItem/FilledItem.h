#pragma once

#include <opencv2/opencv.hpp>

#include "AnimateSDK/app/DOM/Utils/DOMTypes.h"
#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/FilledElement.h"
#include "GraphicItem.h"

namespace sc
{
	namespace Adobe
	{
		class FilledItemContour
		{
		public:
			FilledItemContour(const std::vector<Point2D>& contour) : m_contour(contour)
			{
			}

			const std::vector<Point2D>& Contour() const
			{
				return m_contour;
			}

		public:
			const std::vector<Point2D> m_contour;
		};

		// Class that represents simple filled contour
		class FilledItem : public GraphicItem
		{
		public:
			FilledItem(
				const std::vector<FilledItemContour>& contour,
				const DOM::Utils::COLOR& color,
				const DOM::Utils::MATRIX2D& matrix = { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f }
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
			virtual bool IsFilledShape() const
			{
				return true;
			}

		public:
			const cv::Scalar& Color() const
			{
				return m_color;
			}

			virtual Matrix2x3<float> transformation() const
			{
				return m_matrix;
			}

			const std::vector<FilledItemContour> contours;
		private:
			const Matrix2x3<float> m_matrix;
			cv::Scalar m_color;
		};
	}
}