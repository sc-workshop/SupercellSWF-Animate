#include <FCMPluginInterface.h>

#include "DocType/DocType.h"
#include "DocType/FeatureMatrix.h"

#include "Publisher/Publisher.h"

#include "Utils.h"
#include "Ids.h"

namespace Adobe
{
    BEGIN_MODULE(Module)

        BEGIN_CLASS_ENTRY

        CLASS_ENTRY(CLSID_DocType, ModuleDocumentType)
        CLASS_ENTRY(CLSID_FeatureMatrix, FeatureMatrix)
        CLASS_ENTRY(CLSID_Publisher, sc::Adobe::Publisher)

        END_CLASS_ENTRY

public:
    void SetResPath(const std::string& resPath) { m_resPath = resPath; }
    const std::string& GetResPath() { return m_resPath; }

private:
    std::string m_resPath;

    END_MODULE


        Module module;

    extern "C" FCMPLUGIN_IMP_EXP FCM::Result PluginBoot(FCM::PIFCMCallback callback)
    {
        FCM::Result res;
        std::string langCode;
        std::string modulePath;

        res = module.Init(callback);

        Utils::GetModuleFilePath(modulePath, callback);
        Utils::GetLanguageCode(callback, langCode);

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

        AutoPtr<IFCMDictionary> dict = pPluginDict;

        AutoPtr<IFCMDictionary> plugins;
        dict->AddLevel((const FCM::StringRep8)kFCMComponent, plugins.m_Ptr);

        res = RegisterDocType(plugins, module.GetResPath());
        if (FCM_FAILURE_CODE(res))
        {
            return res;
        }

        res = sc::Adobe::RegisterPublisher(plugins, CLSID_DocType);

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
