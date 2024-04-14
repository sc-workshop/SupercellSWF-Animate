#pragma once

#include <stdint.h>
#include <memory>

#include "AnimateSDK/core/common/FCMTypes.h"
#include "AnimateSDK/core/common/FCMPluginInterface.h"
#include "AnimateSDK/app/DOM/Utils/DOMTypes.h"
#include "AnimateSDK/app/DOM/ILibraryItem.h"

#include "SliceScalingData.h"

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
			SymbolContext(DOM::ILibraryItem* item);
			SymbolContext(const std::u16string& name, SymbolType type);

		public:
			const std::u16string name;
			const SymbolType type = SymbolType::Unknown;

			SliceScalingData slice_scaling;

		private:
			static std::u16string GetName(DOM::ILibraryItem* symbol);
			static SymbolType GetType(DOM::ILibraryItem* symbol);
		};
	}
}