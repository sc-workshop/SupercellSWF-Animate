#include <AnimateSDK/core/common/FCMPluginInterface.h>
#include "AnimateSDK/app/Application/Service/IApplicationService.h"

#include "Document.h"
#include "Publisher.h"

#include "FeatureMatrix/FeatureMatrix.h"

namespace sc {
	namespace Adobe
	{
		BEGIN_MODULE(ModuleInterface)

			BEGIN_CLASS_ENTRY

			CLASS_ENTRY(CLSID_DocType, GenericDocumentType)
			CLASS_ENTRY(CLSID_FeatureMatrix, FeatureMatrix)
			CLASS_ENTRY(CLSID_Publisher, SCPublisher)

			END_CLASS_ENTRY

			END_MODULE

			ModuleInterface Module;

		extern "C" FCMPLUGIN_IMP_EXP
			FCM::Result PluginBoot(FCM::PIFCMCallback callback)
		{
			PluginContext& context = PluginContext::Instance();
			context.logger->info("Called PluginBoot");

			context.UpdateCallback(callback);

			context.logger->info("System Info: ");
			auto application = context.GetService<Application::Service::IApplicationService>(Application::Service::APP_SERVICE);

			{
				FCM::U_Int32 version;
				application->GetVersion(version);
				context.logger->info("	App: Adobe Animate {}.{}.{}.{}",
					((version >> 24) & 0xFF),
					((version >> 16) & 0xFF),
					((version >> 8) & 0xFF),
					((version) & 0xFF)
				);
			}

			context.logger->info("	OS: {}", PluginContext::SystemInfo());

			FCM::Result status = Module.Init(callback);
			context.logger->info("Module inited with status: {}", (uint32_t)status);

			return status;
		}

		extern "C" FCMPLUGIN_IMP_EXP
			FCM::Result PluginGetClassInfo(
				FCM::PIFCMCalloc falloc,
				FCM::PFCMClassInterfaceInfo * info)
		{
			return Module.getClassInfo(falloc, info);
		}

		extern "C" FCMPLUGIN_IMP_EXP
			FCM::Result PluginGetClassObject(
				FCM::PIFCMUnknown unknown,
				FCM::ConstRefFCMCLSID clsid,
				FCM::ConstRefFCMIID iid,
				FCM::PPVoid any)
		{
			return Module.getClassObject(unknown, clsid, iid, any);
		}

		extern "C" FCMPLUGIN_IMP_EXP
			FCM::Result PluginRegister(FCM::PIFCMPluginDictionary pluginDict)
		{
			FCM::Result res = FCM_SUCCESS;

			FCM::AutoPtr<FCM::IFCMDictionary> dict = pluginDict;

			FCM::AutoPtr<FCM::IFCMDictionary> plugins;
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
			FCM::U_Int32 PluginCanUnloadNow(void)
		{
			return Module.canUnloadNow();
		}

		extern "C" FCMPLUGIN_IMP_EXP
			FCM::Result PluginShutdown()
		{
			PluginContext& context = PluginContext::Instance();
			context.logger->info("Called PluginShutdown\n");

			Module.finalize();
			return FCM_SUCCESS;
		}
	};
}