#include "Module/Symbol/SymbolContext.h"
#include "Module/PluginContext.h"

namespace sc
{
	namespace Adobe
	{
		SymbolContext::SymbolContext(DOM::ILibraryItem* item) : name(GetName(item)), type(GetType(item))
		{
		}

		SymbolContext::SymbolContext(const std::u16string& name, SymbolType type) : name(name), type(type)
		{
		}

		std::u16string SymbolContext::GetName(DOM::ILibraryItem* symbol)
		{
			PluginContext& context = PluginContext::Instance();

			FCM::StringRep16 itemNamePtr;
			symbol->GetName(&itemNamePtr);
			std::u16string itemName = (const char16_t*)itemNamePtr;
			context.falloc->Free(itemNamePtr);

			return itemName;
		}

		SymbolContext::SymbolType SymbolContext::GetType(DOM::ILibraryItem* symbol)
		{
			FCM::AutoPtr<FCM::IFCMDictionary> properties;
			symbol->GetProperties(properties.m_Ptr);

			FCM::U_Int32 valueLen;
			FCM::FCMDictRecTypeID propertyType;

			FCM::Result status = properties->GetInfo(kLibProp_SymbolType_DictKey, propertyType, valueLen);
			if (FCM_FAILURE_CODE(status) || propertyType != FCM::FCMDictRecTypeID::kFCMDictType_StringRep8)
			{
				return SymbolContext::SymbolType::Unknown;
			}

			const char* symbolType = (const char*)malloc(valueLen);
			properties->Get(kLibProp_SymbolType_DictKey, propertyType, (FCM::PVoid)symbolType, valueLen);

			SymbolType type = SymbolType::Unknown;
			if (strcmp(symbolType, "MovieClip") == 0)
			{
				type = SymbolType::MovieClip;
			}
			else if (strcmp(symbolType, "Graphic") == 0)
			{
				type = SymbolType::Graphic;
			}

			free((void*)symbolType);

			return type;
		}
	}
}