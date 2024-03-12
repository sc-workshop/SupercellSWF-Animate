#pragma once

#include <stdint.h>
#include <memory>

#include "AnimateSDK/core/common/FCMTypes.h"
#include "AnimateSDK/core/common/FCMPluginInterface.h"
#include "AnimateSDK/app/DOM/Utils/DOMTypes.h"
#include "AnimateSDK/app/DOM/ILibraryItem.h"

namespace sc
{
	namespace Adobe {
		class SymbolContext
		{
		public:
			enum class SymbolType : uint8_t
			{
				Unknown = 0,
				MovieClip,
				Graphic
			};

		public:
			SymbolContext(DOM::ILibraryItem* item) : name(GetName(item)), type(GetType(item))
			{
			}

			SymbolContext(const std::u16string& name, SymbolType type) : name(name), type(type)
			{
			}

			const std::u16string name;
			const SymbolType type = SymbolType::Unknown;

			bool hasSlice9 = false;
			DOM::Utils::RECT slice9 = { {0.0f, 0.0f}, {0.0f, 0.0f} };

		private:
			static std::u16string SymbolContext::GetName(DOM::ILibraryItem* symbol);
			static SymbolType GetType(DOM::ILibraryItem* symbol);
		};
	}
}