#pragma once

#include "core/math/matrix2d.h"
#include "core/hashing/hash_stream.h"
#include "core/hashing/hash.h"

namespace sc
{
	namespace Adobe
	{
		class GraphicItem
		{
		public:
			virtual ~GraphicItem() = default;

		public:
			virtual wk::Matrix2D transformation() const
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