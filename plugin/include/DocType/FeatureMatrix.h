#pragma once

#include "FCMTypes.h"
#include "DocType/IFeatureMatrix.h"
#include "DataTypes.h"
#include "Module/Version.h"
#include "Module/AppContext.h"

#include "JSON.hpp"

using namespace nlohmann;

namespace sc {
	namespace Adobe {
		class FeatureMatrix : public DocType::IFeatureMatrix, public FCM::FCMObjectBase
		{

		public:
			virtual FCM::PIFCMCallback GetCallback() = 0;

			static FCM::U_Int32 GetVersion() { return (FCM::U_Int32)PLUGIN_VERSION; }

			static FCM::FCMInterfaceMap* GetInterfaceMap()
			{
				static FCM::FCMInterfaceMap _pInterfaceMap[] = {
					{
						FCM::IID_IFCMUnknown,
						((FCM::S_Int64)(static_cast<IFCMUnknown*>((FeatureMatrix*)1)) - 1),
						((FCM::_FCM_CREATORARGFUNC*)0)
					},
					{
						DocType::IID_IFeatureMatrix,
						((FCM::S_Int64)(static_cast<IFeatureMatrix*>((FeatureMatrix*)1)) - 1),
						((FCM::_FCM_CREATORARGFUNC*)0)
					},
					{
						FCM::FCMIID_NULL,
						0,
						((FCM::_FCM_CREATORARGFUNC*)0)
					 }
				};

				return _pInterfaceMap;
			}

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

			FCM::Result Init(FCM::PIFCMCallback callback);

			void UpdateContext(FCM::PIFCMCallback callback) {
				m_context = shared_ptr<AppContext>(new AppContext(callback, nullptr, nullptr));
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

			std::shared_ptr<AppContext> m_context;

			friend class FeatureDocumentHandler;
		};
	}
}