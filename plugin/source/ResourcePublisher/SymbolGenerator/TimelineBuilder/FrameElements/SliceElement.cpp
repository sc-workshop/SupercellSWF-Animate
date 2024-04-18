#include "ResourcePublisher/SymbolGenerator/TimelineBuilder/FrameElements/SliceElement.h"

#include "Module/Symbol/SymbolContext.h"

namespace sc
{
	namespace Adobe
	{
		SliceElement::SliceElement(SymbolContext& symbol, FCM::AutoPtr<DOM::FrameElement::IShape> shape, DOM::Utils::MATRIX2D& matrix) :
			transform(matrix), fill(symbol, shape)
		{
		}
	}
}