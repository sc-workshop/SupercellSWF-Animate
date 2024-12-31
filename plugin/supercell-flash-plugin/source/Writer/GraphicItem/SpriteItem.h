#pragma once

#include "GraphicItem.h"

#include "AnimateDOM.h"
#include "AnimatePublisher.h"
#include "core/memory/ref.h"
#include <opencv2/opencv.hpp>
#include <filesystem>

namespace sc
{
	namespace Adobe
	{
		class BitmapItem : public GraphicItem
		{
		public:
			BitmapItem(
				Animate::Publisher::SymbolContext& context,
				wk::Ref<cv::Mat> image, 
				const Animate::DOM::Utils::MATRIX2D& matrix
			);

			BitmapItem(Animate::Publisher::SymbolContext& context);

			virtual ~BitmapItem() = default;

		public:
			virtual const cv::Mat& Image() const { return *m_image; }

			virtual bool IsSprite() const
			{
				return true;
			}

		protected:
			wk::Ref<cv::Mat> m_image;
		};
	}
}