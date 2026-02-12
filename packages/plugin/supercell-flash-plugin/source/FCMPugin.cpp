
#include "AnimateCore.h"
#include "AnimateModule.h"

#include "Module/Module.h"

using namespace sc::Adobe;

static SCPlugin* Module = nullptr;

namespace sc {
	namespace Adobe
	{
		extern "C" FCMPLUGIN_IMP_EXP
			FCM::Result PluginBoot(FCM::PIFCMCallback callback)
		{
			Module = new SCPlugin(callback);
			return FCM_SUCCESS;
		}

		extern "C" FCMPLUGIN_IMP_EXP
			FCM::Result PluginGetClassInfo(
				FCM::PIFCMCalloc falloc,
				FCM::PFCMClassInterfaceInfo* info)
		{
#if WK_DEBUG
            Module->logger->info("Called get PluginGetClassInfo");
#endif
			return Module->GetClassInfo(falloc, info);
		}

		extern "C" FCMPLUGIN_IMP_EXP
			FCM::Result PluginGetClassObject(
				FCM::PIFCMUnknown,
				FCM::ConstRefFCMCLSID clsid,
				FCM::ConstRefFCMIID iid,
				FCM::PPVoid any)
		{
#if WK_DEBUG
            std::string clsid_name = clsid;
            std::string iid_name = iid;
            Module->logger->info("Called get PluginGetClassObject clsid: {}, iid: {}", clsid_name.c_str(), iid_name.c_str());
#endif
			return Module->GetClassObject(clsid, iid, any);
		}
		
		extern "C" FCMPLUGIN_IMP_EXP
			FCM::Result PluginRegister(FCM::PIFCMPluginDictionary pluginDict)
		{
			return Module->RegisterPlugin<SCPlugin::Publisher, SCPlugin::DocType>(pluginDict);
		}

		extern "C" FCMPLUGIN_IMP_EXP
			FCM::U_Int32 PluginCanUnloadNow(void)
		{
			return Module->CanUnloadNow();
		}

		extern "C" FCMPLUGIN_IMP_EXP
			FCM::Result PluginShutdown()
		{
			SCPlugin& context = SCPlugin::Instance();
			context.logger->info("Called PluginShutdown\n");

			Module->Finalize();

			return FCM_SUCCESS;
		}
	};
}
