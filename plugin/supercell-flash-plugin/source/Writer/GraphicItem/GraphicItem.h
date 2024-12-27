#pragma once

#include "core/math/matrix2d.h"
#include "core/hashing/hash_stream.h"
#include "core/hashing/hash.h"
#include "core/exception/exception.h"
#include "opencv2/opencv.hpp"

namespace sc
{
	namespace Adobe
	{
		class GraphicItem
		{
		public:
			virtual ~GraphicItem() = default;

		public:
			virtual wk::Matrix2D Transformation() const
			{
				// Returns none transform
				return {};
			}

		public:
			virtual bool IsSprite() const
			{
				return false;
			}

			virtual bool IsSliced() const
			{
				return false;
			}

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
		};
	}
}