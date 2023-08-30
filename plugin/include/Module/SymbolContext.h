#pragma once

#include "DOM/Utils/DOMTypes.h"

namespace sc
{
	namespace Adobe {
		struct SymbolContext
		{
			std::u16string name;
			std::string type;

			bool hasSlice9 = false;
			DOM::Utils::RECT slice9{ 0 };
		};
	}
}