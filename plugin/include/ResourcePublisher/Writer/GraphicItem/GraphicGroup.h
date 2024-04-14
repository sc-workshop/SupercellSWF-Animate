#pragma once

#include <vector>

#include "GraphicItem.h"
#include "Module/Symbol/SymbolContext.h"

#include "generic/ref.h"

namespace sc
{
	namespace Adobe
	{
		class GraphicGroup
		{
		public:
			GraphicGroup(SymbolContext& context);

		public:
			SymbolContext symbol;

		public:
			template<typename T, typename ... Args>
			T& AddItem(Args&& ... args)
			{
				Ref<T> item = CreateRef<T>(std::forward<Args>(args)...);
				m_items.push_back(item);
				return *item;
			}

			GraphicItem& getItem(size_t index);
			size_t size() const;

		private:
			std::vector<Ref<GraphicItem>> m_items;
		};
	}
}