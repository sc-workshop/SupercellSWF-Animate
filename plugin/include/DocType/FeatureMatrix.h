#pragma once

#include "FCMTypes.h"
#include "DocType/IFeatureMatrix.h"
#include "DataTypes.h"

#include "Version.h"

using namespace FCM;
using namespace DocType;

namespace SupercellSWF {
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

		FCM::Result StartElement(
			const std::string name,
			const std::map<std::string, std::string>& attrs);

		FCM::Result EndElement(const std::string name);

		Feature* FindFeature(const std::string& inFeatureName);

		Feature* UpdateFeature(const std::map<std::string, std::string>& inAttrs);

		Property* UpdateProperty(Feature* inFeature, const std::map<std::string, std::string>& inAttrs);

		Value* UpdateValue(Property* inProperty, const std::map<std::string, std::string>& inAttrs);

	private:
		StrFeatureMap m_features;
		Feature* m_currentFeature;
		Property* m_currentProperty;
		bool m_inited = false;

		friend class FeatureDocumentHandler;
	};
}