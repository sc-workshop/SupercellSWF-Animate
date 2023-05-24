#include <FCMPluginInterface.h>

#include "DocType/DocType.h"
#include "DocType/FeatureMatrix.h"

#include "Publisher/Publisher.h"

#include "Utils.h"
#include "Ids.h"
#include "Localization.h"

namespace sc {
	namespace Adobe
	{
		BEGIN_MODULE(ModuleInterface)

		BEGIN_CLASS_ENTRY

			CLASS_ENTRY(CLSID_DocType, DocumentType)
			CLASS_ENTRY(CLSID_FeatureMatrix, FeatureMatrix)
			CLASS_ENTRY(CLSID_Publisher, Publisher)

		END_CLASS_ENTRY

		END_MODULE

		ModuleInterface Module;

		extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginBoot(FCM::PIFCMCallback callback)
		{
			FCM::Result res;
			std::string langCode;
			std::string modulePath;

			res = Module.Init(callback);

			Utils::GetModuleFilePath(modulePath, callback);
			Utils::GetLanguageCode(langCode, callback);

			Locale.Load(Utils::GetPath(modulePath), langCode);

			return res;
		}

		extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginGetClassInfo(
			FCM::PIFCMCalloc pCalloc,
			FCM::PFCMClassInterfaceInfo * ppClassInfo)
		{
			return Module.getClassInfo(pCalloc, ppClassInfo);
		}

		extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginGetClassObject(
			FCM::PIFCMUnknown pUnkOuter,
			FCM::ConstRefFCMCLSID clsid,
			FCM::ConstRefFCMIID iid,
			FCM::PPVoid pAny)
		{
			return Module.getClassObject(pUnkOuter, clsid, iid, pAny);
		}

		extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginRegister(FCM::PIFCMPluginDictionary pPluginDict)
		{
			FCM::Result res = FCM_SUCCESS;

			AutoPtr<IFCMDictionary> dict = pPluginDict;

			AutoPtr<IFCMDictionary> plugins;
			dict->AddLevel((const FCM::StringRep8)kFCMComponent, plugins.m_Ptr);

			res = RegisterDocType(plugins);
			if (FCM_FAILURE_CODE(res))
			{
				return res;
			}

			res = sc::Adobe::RegisterPublisher(plugins, CLSID_DocType);

			return res;
		}

		extern "C" FCMPLUGIN_IMP_EXP FCM::U_Int32 PluginCanUnloadNow(void)
		{
			return Module.canUnloadNow();
		}

		extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginShutdown()
		{
			Module.finalize();

			return FCM_SUCCESS;
		}
	};
}