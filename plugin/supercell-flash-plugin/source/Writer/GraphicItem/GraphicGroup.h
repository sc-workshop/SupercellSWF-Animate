#pragma once

#include "AnimatePublisher.h"
#include "core/memory/ref.h"
#include <vector>

#include "GraphicItem.h"
#include "FilledItem.h"
#include "SpriteItem.h"
#include "SlicedItem.h"



namespace sc
{
	namespace Adobe
	{
		class GraphicGroup
		{
		public:
			GraphicGroup(Animate::Publisher::SymbolContext& context);

		public:
			Animate::Publisher::SymbolContext symbol;

		public:
			template<typename T, typename ... Args>
			T& AddItem(Args&& ... args)
			{
				wk::Ref<T> item = wk::CreateRef<T>(std::forward<Args>(args)...);
				m_items.push_back(item);
				return *item;
			}

			GraphicItem& GetItem(size_t index) const;
			size_t Size() const;

		private:
			std::vector<wk::Ref<GraphicItem>> m_items;
		};
	}
}