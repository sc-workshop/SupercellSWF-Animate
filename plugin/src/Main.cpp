#include <FCMPluginInterface.h>

#include "DocType/DocType.h"
#include "DocType/FeatureMatrix.h"

#include "Publisher/Publisher.h"

using namespace FCM;

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

		extern "C" FCMPLUGIN_IMP_EXP 
		Result PluginBoot(PIFCMCallback callback)
		{
			return Module.Init(callback);
		}

		extern "C" FCMPLUGIN_IMP_EXP 
		Result PluginGetClassInfo(
			PIFCMCalloc falloc,
			PFCMClassInterfaceInfo* info)
		{
			return Module.getClassInfo(falloc, info);
		}

		extern "C" FCMPLUGIN_IMP_EXP 
		Result PluginGetClassObject(
			FCM::PIFCMUnknown unknown,
			FCM::ConstRefFCMCLSID clsid,
			FCM::ConstRefFCMIID iid,
			FCM::PPVoid any)
		{
			return Module.getClassObject(unknown, clsid, iid, any);
		}

		extern "C" FCMPLUGIN_IMP_EXP 
		Result PluginRegister(FCM::PIFCMPluginDictionary pluginDict)
		{
			FCM::Result res = FCM_SUCCESS;

			AutoPtr<IFCMDictionary> dict = pluginDict;

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

		extern "C" FCMPLUGIN_IMP_EXP 
		U_Int32 PluginCanUnloadNow(void)
		{
			return Module.canUnloadNow();
		}

		extern "C" FCMPLUGIN_IMP_EXP 
		Result PluginShutdown()
		{
			Module.finalize();

			return FCM_SUCCESS;
		}
	};
}