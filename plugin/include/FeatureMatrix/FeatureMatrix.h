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
#include "FeaturesTypes.h"
#include "Utils.h"

#include "json.hpp"
using namespace nlohmann;

using namespace FCM;
using namespace DocType;

namespace sc {
	namespace Adobe {
		class FeatureMatrix : public DocType::IFeatureMatrix, public FCM::FCMObjectBase
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

			~FeatureMatrix();

			void Init(FCM::PIFCMCallback pCallback);

		private:
			void ReadFeature(json& feature);
			void ReadProperty(Feature& feature, json& property);
			void ReadValue(Property& property, json& value);

			Feature* FindFeature(const std::string& inFeatureName);

		private:
			FCM::IFCMCallback* m_callback = nullptr;
			FeatureMap m_features;
		};
	}
}