#pragma once

#include "math/matrix2x3.h"

namespace sc
{
	namespace Adobe
	{
		class GraphicItem
		{
		public:
			virtual ~GraphicItem() = default;

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

			virtual bool IsSliced() const
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