#include "ResourcePublisher/Writer/GraphicItem/GraphicGroup.h"

namespace sc
{
	namespace Adobe
	{
		GraphicGroup::GraphicGroup(SymbolContext& context) : symbol(context)
		{
		};

		GraphicItem& GraphicGroup::getItem(size_t index)
		{
			return *m_items[index];
		}

		size_t GraphicGroup::size() const
		{
			return m_items.size();
		}
	}
}