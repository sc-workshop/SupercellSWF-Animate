#pragma once

#include "AnimatePublisher.h"
#include "opencv2/opencv.hpp"
#include "core/exception/exception.h"
#include "core/math/matrix2d.h"

namespace sc::Adobe
{
	class GraphicItem : public Animate::Publisher::StaticElement
	{
	public:
		GraphicItem(Animate::Publisher::SymbolContext& context) : Animate::Publisher::StaticElement(context) {};

	public:
		virtual bool IsSolidColor() const
		{
			return false;
		}

		virtual const cv::Mat& Image() const
		{
			throw wk::Exception();
		}

		virtual const cv::Scalar& Color() const
		{
			throw wk::Exception();
		}

		wk::Matrix2D Transformation2D() const
		{
			auto m = Transformation();

			return wk::Matrix2D(
				m.a, m.b, m.c, m.d,
				m.tx, m.ty
			);
		}
	};
}