#pragma once

#include "GraphicItem.h"

#include "AnimateDOM.h"
#include "AnimatePublisher.h"
#include "core/memory/ref.h"
#include "core/image/raw_image.h"
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
				wk::RawImageRef image,
				const Animate::DOM::Utils::MATRIX2D& matrix
			);

			BitmapItem(Animate::Publisher::SymbolContext& context);

			virtual ~BitmapItem() = default;

		public:
			virtual const wk::RawImage& Image() const { return *m_image; }

			virtual bool IsSprite() const
			{
				return true;
			}

		protected:
			wk::RawImageRef m_image;
		};
	}
}