#pragma once

#include <string>
#include <map>
#include <fstream>
#include <sstream>

#include "AnimateSDK/core/common/FCMTypes.h"
#include "AnimateSDK/core/common/FCMPluginInterface.h"
#include "AnimateSDK/app/DocType/IDocType.h"
#include "AnimateSDK/app/ApplicationFCMPublicIDs.h"
#include "AnimateSDK/app/Application/Service/IOutputConsoleService.h"

#include "Module/Localization.h"
#include "Module/PluginContext.h"
#include "Module/Version.h"
#include "FeaturesTypes.h"

#include <nlohmann/json.hpp>
using namespace nlohmann;

using namespace DocType;

namespace sc {
	namespace Adobe {
		class FeatureMatrix : public FCM::FCMObjectBase, public DocType::IFeatureMatrix
		{
			BEGIN_MULTI_INTERFACE_MAP(FeatureMatrix, PLUGIN_VERSION)
				INTERFACE_ENTRY(IFeatureMatrix)
				END_INTERFACE_MAP

		public:
			virtual FCM::Result _FCMCALL IsSupported(
				FCM::CStringRep16 inFeatureName,
				FCM::Boolean& isSupported);

			virtual FCM::Result _FCMCALL IsSupported(
				FCM::CStringRep16 inFeatureName,
				FCM::CStringRep16 inPropName,
				FCM::Boolean& isSupported);

			virtual FCM::Result _FCMCALL IsSupported(
				FCM::CStringRep16 inFeatureName,
				FCM::CStringRep16 inPropName,
				FCM::CStringRep16 inValName,
				FCM::Boolean& isSupported);

			virtual FCM::Result _FCMCALL GetDefaultValue(
				FCM::CStringRep16 inFeatureName,
				FCM::CStringRep16 inPropName,
				FCM::VARIANT& outDefVal);

			FeatureMatrix();
			virtual ~FeatureMatrix();

		private:
			void ReadFeature(json& feature);
			void ReadProperty(Feature& feature, json& property);
			void ReadValue(Property& property, json& value);

			Feature* FindFeature(const std::string& inFeatureName);

		private:
#if SC_DEBUG
			json m_dump = json::object();
#endif
			FeatureMap m_features;
		};
	}
}