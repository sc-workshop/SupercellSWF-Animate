#pragma once

#include <string>
#include <map>
#include <fstream>
#include <sstream>

#include "FCMTypes.h"
#include "FCMPluginInterface.h"
#include "DocType/IDocType.h"
#include "ApplicationFCMPublicIDs.h"
#include "Application/Service/IOutputConsoleService.h"

#include "Module/Version.h"
#include "Document/FeaturesTypes.h"
#include "Utils.h"

#include "JSON.hpp"
using namespace nlohmann;

using namespace FCM;
using namespace DocType;

namespace sc {
	namespace Adobe {
        class FeatureDocumentHandler;

        //static const std::string kElement_Features("Features");
        //static const std::string kElement_Feature("Feature");
        //static const std::string kElement_Property("Property");
        //static const std::string kElement_Value("Value");
        //static const std::string kAttribute_name("name");
        //static const std::string kAttribute_supported("supported");
        //static const std::string kAttribute_default("default");
        //static const std::string kValue_true("true");
        //static const std::string kValue_false("false");

        class FeatureMatrix : public DocType::IFeatureMatrix, public FCM::FCMObjectBase
        {
            BEGIN_MULTI_INTERFACE_MAP(FeatureMatrix, PLUGIN_VERSION)
                INTERFACE_ENTRY(IFeatureMatrix)
            END_INTERFACE_MAP

        public:

            virtual FCM::Result _FCMCALL IsSupported(
                CStringRep16 inFeatureName,
                FCM::Boolean& isSupported);

            virtual FCM::Result _FCMCALL IsSupported(
                CStringRep16 inFeatureName,
                CStringRep16 inPropName,
                FCM::Boolean& isSupported);

            virtual FCM::Result _FCMCALL IsSupported(
                CStringRep16 inFeatureName,
                CStringRep16 inPropName,
                CStringRep16 inValName,
                FCM::Boolean& isSupported);

            virtual FCM::Result _FCMCALL GetDefaultValue(
                CStringRep16 inFeatureName,
                CStringRep16 inPropName,
                FCM::VARIANT& outDefVal);

            FeatureMatrix();

            ~FeatureMatrix();

            void Init(FCM::PIFCMCallback pCallback);

        private:
            void ReadFeature(json& feature);
            void ReadProperty(Feature& feature, json& property);
            void ReadValue(Property& property, json& value);

            Feature* FindFeature(const std::string& inFeatureName);

        private:
            PIFCMCallback m_callback = nullptr;
            FeatureMap m_features;

            friend class FeatureDocumentHandler;
        };
	}
}