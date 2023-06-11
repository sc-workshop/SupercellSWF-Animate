#pragma once

#include "FCMTypes.h"
#include "DocType/IFeatureMatrix.h"
#include "DataTypes.h"
#include "Module/Version.h"
#include "Module/AppContext.h"

#include "JSON.hpp"

using namespace FCM;
using namespace DocType;
using namespace nlohmann;

namespace sc {
	namespace Adobe {
		class FeatureMatrix : public IFeatureMatrix, public FCMObjectBase
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

			FCM::Result Init(FCM::PIFCMCallback callback);

			void UpdateContext(FCM::PIFCMCallback callback) {
				m_context = shared_ptr<AppContext>(new AppContext(callback, nullptr));
			}

		private:

			FCM::Result StartElement(
				const std::string name,
				const std::map<std::string, std::string>& attrs);

			FCM::Result EndElement(const std::string name);

			Feature* FindFeature(const std::string& inFeatureName);

			Feature* UpdateFeature(const std::map<std::string, std::string>& inAttrs);

			Property* UpdateProperty(Feature* inFeature, const std::map<std::string, std::string>& inAttrs);

			Value* UpdateValue(Property* inProperty, const std::map<std::string, std::string>& inAttrs);

			void ReadFeature(json& feature);
			void ReadProperty(Feature& feature, json& property);
			void ReadValue(Property& property, json& value);

		private:
			FeatureMap m_features;
			Feature* m_currentFeature;
			Property* m_currentProperty;

			shared_ptr<AppContext> m_context;

			friend class FeatureDocumentHandler;
		};
	}
}