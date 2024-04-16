#pragma once

#include "generic/ref.h"
#include "GraphicItem.h"
#include "AnimateSDK/app/DOM/Utils/DOMTypes.h"

#include <opencv2/opencv.hpp>

#include <filesystem>

namespace sc
{
	namespace Adobe
	{
		class SpriteItem : public GraphicItem
		{
		public:
			SpriteItem(Ref<cv::Mat> image, const DOM::Utils::MATRIX2D& matrix);

		public:
			virtual Matrix2x3<float> transformation() const
			{
				return m_matrix;
			}

			virtual bool IsSprite() const
			{
				return true;
			}

		public:
			cv::Mat& image() { return *m_image; }

		private:
			static Matrix2x3<float> init_matrix(const DOM::Utils::MATRIX2D& matrix);

		private:
			Ref<cv::Mat> m_image;
			Matrix2x3<float> m_matrix;
		};
	}
}