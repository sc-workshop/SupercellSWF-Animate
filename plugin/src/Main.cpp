#include <FCMPluginInterface.h>

#include "DocType/DocType.h"
#include "DocType/FeatureMatrix.h"

#include "SharedPublisher/Publisher.h"
#include "SharedPublisher/ResourcePalette.h"
#include "SharedPublisher/TimelineBuilder.h"
#include "SharedPublisher/TimelineBuilderFactory.h"

#include "Utils.h"
#include "Ids.h"

namespace SupercellSWF
{
    BEGIN_MODULE(Module)

        BEGIN_CLASS_ENTRY

        CLASS_ENTRY(CLSID_DocType, ModuleDocumentType)
        CLASS_ENTRY(CLSID_FeatureMatrix, FeatureMatrix)
        CLASS_ENTRY(CLSID_Publisher, SharedPublisher)
        CLASS_ENTRY(CLSID_ResourcePalette, ResourcePalette)
        CLASS_ENTRY(CLSID_TimelineBuilder, TimelineBuilder)
        CLASS_ENTRY(CLSID_TimelineBuilderFactory, TimelineBuilderFactory)

        END_CLASS_ENTRY

public:
    void SetResPath(const std::string& resPath) { m_resPath = resPath; }
    const std::string& GetResPath() { return m_resPath; }

private:
    std::string m_resPath;

    END_MODULE


        Module module;

    extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginBoot(FCM::PIFCMCallback pCallback)
    {
        FCM::Result res;
        std::string langCode;
        std::string modulePath;

        res = module.init(pCallback);

        Utils::GetModuleFilePath(modulePath, pCallback);
        Utils::GetLanguageCode(pCallback, langCode);

        module.SetResPath(modulePath + "../res/" + langCode + "/");
        return res;
    }

    extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginGetClassInfo(
        FCM::PIFCMCalloc pCalloc,
        FCM::PFCMClassInterfaceInfo * ppClassInfo)
    {
        return module.getClassInfo(pCalloc, ppClassInfo);
    }

    extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginGetClassObject(
        FCM::PIFCMUnknown pUnkOuter,
        FCM::ConstRefFCMCLSID clsid,
        FCM::ConstRefFCMIID iid,
        FCM::PPVoid pAny)
    {
        return module.getClassObject(pUnkOuter, clsid, iid, pAny);
    }

    // Register the plugin - Register plugin as both DocType and Publisher
    extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginRegister(FCM::PIFCMPluginDictionary pPluginDict)
    {
        FCM::Result res = FCM_SUCCESS;

        AutoPtr<IFCMDictionary> pDictionary = pPluginDict;

        AutoPtr<IFCMDictionary> pPlugins;
        pDictionary->AddLevel((const FCM::StringRep8)kFCMComponent, pPlugins.m_Ptr);

        res = RegisterDocType(pPlugins, module.GetResPath());
        if (FCM_FAILURE_CODE(res))
        {
            return res;
        }

        res = RegisterPublisher(pPlugins, CLSID_DocType);

        return res;
    }

    extern "C" FCMPLUGIN_IMP_EXP FCM::U_Int32 PluginCanUnloadNow(void)
    {
        return module.canUnloadNow();
    }

    extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginShutdown()
    {
        module.finalize();

        return FCM_SUCCESS;
    }

};
