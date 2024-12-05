#include "GraphicGroup.h"

using namespace Animate::Publisher;

namespace sc
{
	namespace Adobe
	{
		GraphicGroup::GraphicGroup(SymbolContext& context) : symbol(context)
		{
		};

		GraphicItem& GraphicGroup::GetItem(size_t index)
		{
			return *m_items[index];
		}

		size_t GraphicGroup::Size() const
		{
			return m_items.size();
		}
	}
}