#pragma once

#include "math/matrix2x3.h"

namespace sc
{
	namespace Adobe
	{
		class SpriteItem;

		class GraphicItem
		{
		public:
			virtual Matrix2x3<float> transformation() const
			{
				// Returns none transform
				return {};
			}

		public:
			virtual bool IsSprite() const
			{
				return false;
			}

			virtual bool IsFilledShape() const
			{
				return false;
			}
		};
	}
}