#pragma once

#include "GraphicItem.h"

#include "AnimateDOM.h"
#include <opencv2/opencv.hpp>
#include <filesystem>

namespace sc
{
	namespace Adobe
	{
		class SpriteItem : public GraphicItem
		{
		public:
			SpriteItem(wk::Ref<cv::Mat> image, const Animate::DOM::Utils::MATRIX2D& matrix);
			virtual ~SpriteItem() = default;

		public:
			virtual wk::Matrix2D transformation() const
			{
				return m_matrix;
			}

			virtual bool IsSprite() const
			{
				return true;
			}

		public:
			cv::Mat& Image() const { return *m_image; }

		private:
			static wk::Matrix2D init_matrix(const Animate::DOM::Utils::MATRIX2D& matrix);

		private:
			wk::Ref<cv::Mat> m_image;
			wk::Matrix2D m_matrix;
		};
	}
}