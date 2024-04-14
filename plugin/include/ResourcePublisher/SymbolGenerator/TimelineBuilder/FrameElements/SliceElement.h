#pragma once

#include "AnimateSDK/app/DOM/Utils/DOMTypes.h"
#include "FilledElement.h"

namespace sc
{
	namespace Adobe
	{
		class SymbolContext;

		class SliceElement
		{
		public:
			SliceElement(
				SymbolContext& symbol,
				FCM::AutoPtr<DOM::FrameElement::IShape> shape,
				DOM::Utils::MATRIX2D& matrix
			);

		public:
			DOM::Utils::MATRIX2D transform;
			FilledElement fill;
		};
	}
}