#include "DocType/FeatureMatrix.h"

#include "PluginConfiguration.h"
#include "Utils.h"

#include <Application/Service/IOutputConsoleService.h>
#include <FlashFCMPublicIDs.h>

#include <fstream>
#include <sstream>

#include <filesystem>
namespace fs = std::filesystem;

namespace sc {
	namespace Adobe {
		static const std::string kElement_Features("Features");
		static const std::string kElement_Feature("Feature");
		static const std::string kElement_Property("Property");
		static const std::string kElement_Value("Value");
		static const std::string kAttribute_name("name");
		static const std::string kAttribute_supported("supported");
		static const std::string kAttribute_default("default");
		static const std::string kValue_true("true");
		static const std::string kValue_false("false");

		FeatureMatrix::FeatureMatrix()
		{
		}

		FeatureMatrix::~FeatureMatrix()
		{
		}

		Result FeatureMatrix::Init(FCM::PIFCMCallback callback)
		{
			std::string modulePath;
			sc::Adobe::Utils::GetModuleFilePath(modulePath, callback);

			console.Init("Features", callback);

			std::string libPath;
			sc::Adobe::Utils::GetParent(modulePath, libPath);

			fs::path featuresPath(libPath);
			featuresPath /= "res";
			featuresPath /= "Features.json";

			std::ifstream file(featuresPath);

			size_t fileSize = 0;

			file.seekg(0, file.end);
			fileSize = file.tellg();
			file.seekg(0, file.beg);

			if (fileSize <= 0) {
				console.log("Failed to get feature list. All scene features are disabled.");
				return FCM_GENERAL_ERROR;
			};

			std::string featureListBuffer(fileSize, ' ');

			file.read(featureListBuffer.data(), fileSize);
			file.close();

			JSONNode features = libjson::parse(featureListBuffer);
			for (JSONNode feature : features) {
				ReadFeature(feature);
			}

			return FCM_SUCCESS;
		}

		void FeatureMatrix::ReadFeature(JSONNode& feature) {
			std::string featureName = "";
			bool featureSupported = false;
			JSONNode propertiesNode;

			for (JSONNode featureVariable : feature) {
				std::string featureVariableName = featureVariable.name();

				if (featureVariableName == "name") {
					featureName = featureVariable.as_string();
				}
				else if (featureVariableName == "supported") {
					featureSupported = featureVariable.as_bool();
				}
				else if (featureVariableName == "properties") {
					propertiesNode = featureVariable.as_array();
				}
			}

			if (featureName.empty()) return;

			Feature featureItem(featureSupported);

			if (!propertiesNode.empty()) {
				for (JSONNode property : propertiesNode) {
					ReadProperty(featureItem, property);
				}
			}

			m_features.insert(std::pair(featureName, featureItem));
		}

		void FeatureMatrix::ReadProperty(Feature& feature, JSONNode& property) {
			std::string propertyName = "";
			std::string propertyDefault = "";
			bool propertySupported = false;

			JSONNode valuesNode;

			for (JSONNode propertyVariable : property) {
				std::string propertyVariableName = propertyVariable.name();

				if (propertyVariableName == "name") {
					propertyName = propertyVariable.as_string();
				}
				else if (propertyVariableName == "supported") {
					propertySupported = propertyVariable.as_bool();
				}
				else if (propertyVariableName == "default") {
					propertyDefault = propertyVariable.as_string();
				}
				else if (propertyVariableName == "values") {
					valuesNode = propertyVariable.as_array();
				}
			}

			Property propertyItem(propertyDefault, propertySupported);

			if (!valuesNode.empty()) {
				for (JSONNode value : valuesNode) {
					ReadValue(propertyItem, value);
				}
			}

			if (!propertyName.empty()) {
				feature.AddProperty(propertyName, propertyItem);
			}
		}

		void FeatureMatrix::ReadValue(Property& property, JSONNode& value) {
			std::string valueName = "";
			bool valueSupported = false;

			for (JSONNode valueVariable : value) {
				std::string valueVariableName = valueVariable.name();

				if (valueVariableName == "name") {
					valueName = valueVariable.as_string();
				}
				else if (valueVariableName == "supported") {
					valueSupported = valueVariable.as_bool();
				}
			}

			if (!valueName.empty()) {
				property.AddValue(valueName, Value(valueSupported));
			}
		}

		FCM::Result FeatureMatrix::IsSupported(CStringRep16 inFeatureName, FCM::Boolean& isSupported)
		{
			std::string featureName = sc::Adobe::Utils::ToString(inFeatureName, GetCallback());

			if (featureName.empty()) {
				return true;
			}

			Feature* feature = FindFeature(featureName);
			if (feature == NULL)
			{
				console.log("Failed to get info about \"%s\" feature", featureName.c_str());
				isSupported = false;
			}
			else
			{
				isSupported = feature->IsSupported();
			}
			return FCM_SUCCESS;
		}

		FCM::Result FeatureMatrix::IsSupported(
			CStringRep16 inFeatureName,
			CStringRep16 inPropName,
			FCM::Boolean& isSupported)
		{
			std::string featureName = sc::Adobe::Utils::ToString(inFeatureName, GetCallback());
			std::string propertyName = sc::Adobe::Utils::ToString(inPropName, GetCallback());

			if (featureName.empty() || propertyName.empty()) {
				return true;
			}

			Feature* feature = FindFeature(featureName);
			if (feature == NULL)
			{
				console.log("Failed to get info about \"%s\" feature", featureName.c_str());
				isSupported = false;
			}
			else
			{
				if (!feature->IsSupported())
				{
					isSupported = false;
				}
				else
				{
					Property* property = feature->FindProperty(propertyName);
					if (property == NULL)
					{
						console.log("Failed to get property \"%s\" from \"%s\" feature", propertyName.c_str(), featureName.c_str());
						isSupported = false;
					}
					else
					{
						isSupported = property->IsSupported();
					}
				}
			}
			return FCM_SUCCESS;
		}

		FCM::Result FeatureMatrix::IsSupported(
			CStringRep16 inFeatureName,
			CStringRep16 inPropName,
			CStringRep16 inValName,
			FCM::Boolean& isSupported)
		{
			std::string featureName = sc::Adobe::Utils::ToString(inFeatureName, GetCallback());
			std::string propertyName(sc::Adobe::Utils::ToString(inPropName, GetCallback()));
			std::string valueName(sc::Adobe::Utils::ToString(inValName, GetCallback()));

			if (featureName.empty() || propertyName.empty() || valueName.empty()) {
				return true;
			}

			Feature* feature = FindFeature(featureName);
			if (feature == NULL)
			{
				console.log("Failed to get info about \"%s\" feature", featureName.c_str());
				isSupported = false;
			}
			else
			{
				if (!feature->IsSupported())
				{
					isSupported = false;
				}
				else
				{
					Property* property = feature->FindProperty(propertyName);
					if (property == NULL)
					{
						console.log("Failed to get property \"%s\" from \"%s\" feature", propertyName.c_str(), featureName.c_str());
						isSupported = false;
					}
					else
					{
						if (!property->IsSupported())
						{
							isSupported = false;
						}
						else
						{
							Value* value = property->FindValue(valueName);
							if (!valueName.empty() && value == NULL)
							{
								console.log("Failed to get value \"%s\" from \"%s\" property in \"%s\" feature", valueName.c_str(), propertyName.c_str(), featureName.c_str());
								isSupported = false;
							}
							else
							{
								isSupported = value->IsSupported();
							}
						}
					}
				}
			}
			return FCM_SUCCESS;
		}

		FCM::Result FeatureMatrix::GetDefaultValue(CStringRep16 inFeatureName,
			CStringRep16 inPropName,
			FCM::VARIANT& outDefVal)
		{
			// Any boolean value retuened as string should be "true" or "false"
			FCM::Result res = FCM_INVALID_PARAM;
			std::string featureName = sc::Adobe::Utils::ToString(inFeatureName, GetCallback());
			std::string propName = sc::Adobe::Utils::ToString(inPropName, GetCallback());

			Property* pProperty = NULL;
			Feature* pFeature = FindFeature(featureName);
			if (pFeature != NULL && pFeature->IsSupported())
			{
				pProperty = pFeature->FindProperty(propName);
				if (pProperty != NULL /*&& pProperty->IsSupported()*/)
				{
					std::string strVal = pProperty->GetDefault();
					std::istringstream iss(strVal);
					res = FCM_SUCCESS;
					switch (outDefVal.m_type) {
					case kFCMVarype_UInt32: iss >> outDefVal.m_value.uVal; break;
					case kFCMVarype_Float: iss >> outDefVal.m_value.fVal; break;
					case kFCMVarype_Bool: outDefVal.m_value.bVal = (kValue_true == strVal); break;
					case kFCMVarype_CString: outDefVal.m_value.strVal = sc::Adobe::Utils::ToString16(strVal, GetCallback()); break;
					case kFCMVarype_Double: iss >> outDefVal.m_value.dVal; break;
					default:
						ASSERT(0);
						res = FCM_INVALID_PARAM;
						break;
					}
				}
			}

			return res;
		}

		FCM::Result FeatureMatrix::StartElement(
			const std::string name,
			const std::map<std::string, std::string>& attrs)
		{
			std::string name8(name);

			if (kElement_Feature.compare(name8) == 0)
			{
				// Start of a feature tag
				m_currentFeature = UpdateFeature(attrs);
				m_currentProperty = NULL;
			}
			else if (kElement_Property.compare(name8) == 0)
			{
				// Start of a property tag
				m_currentProperty = UpdateProperty(m_currentFeature, attrs);
			}
			else if (kElement_Value.compare(name8) == 0)
			{
				// Start of a value tag
				UpdateValue(m_currentProperty, attrs);
			}

			return FCM_SUCCESS;
		}

		FCM::Result FeatureMatrix::EndElement(const std::string name)
		{
			std::string name8(name);

			if (kElement_Feature.compare(name8) == 0)
			{
				// End of a feature tag
				m_currentFeature = NULL;
				m_currentProperty = NULL;
			}
			else if (kElement_Property.compare(name8) == 0)
			{
				// End of a property tag
				m_currentProperty = NULL;
			}
			return FCM_SUCCESS;
		}

		Feature* FeatureMatrix::FindFeature(const std::string& inFeatureName)
		{
			FeatureMap::iterator itr = m_features.find(inFeatureName);
			if (itr != m_features.end())
			{
				return &itr->second;
			}
			return NULL;
		}

		Feature* FeatureMatrix::UpdateFeature(const std::map<std::string, std::string>& inAttrs)
		{
			// name: mandatory attribute
			std::string name;
			std::map<std::string, std::string>::const_iterator itr = inAttrs.find(kAttribute_name);
			if ((itr == inAttrs.end()) || (itr->second.empty()))
			{
				return NULL;
			}
			else
			{
				name = itr->second;
			}

			// supported: optional attribute
			bool supported = true;
			itr = inAttrs.find(kAttribute_supported);
			if (itr != inAttrs.end())
			{
				supported = (itr->second == kValue_true);
			}

			// Find or Create new Feature

			Feature* pFeature = FindFeature(name);
			if (pFeature == NULL)
			{
				pFeature = new Feature(supported);
				m_features.insert(std::pair<std::string, Feature*>(name, pFeature));
			}

			return pFeature;
		}

		Property* FeatureMatrix::UpdateProperty(
			Feature* inFeature,
			const std::map<std::string, std::string>& inAttrs)
		{
			if (inFeature == NULL)
			{
				return NULL;
			}

			std::string name;

			// name: mandatory attribute
			std::map<std::string, std::string>::const_iterator itr = inAttrs.find(kAttribute_name);
			if ((itr == inAttrs.end()) || (itr->second.empty()))
			{
				return NULL;
			}
			else
			{
				name = itr->second;
			}

			// supported: optional attribute
			bool supported = true;
			itr = inAttrs.find(kAttribute_supported);
			if (itr != inAttrs.end())
			{
				supported = itr->second == kValue_true;
			}

			// default: optional attribute
			std::string def;
			itr = inAttrs.find(kAttribute_default);
			if ((itr != inAttrs.end()) && (itr->second.empty() == false))
			{
				def = itr->second;
			}

			// Find or Create new Property
			Property* property = NULL;
			property = inFeature->FindProperty(name);
			if (property == NULL)
			{
				property = new Property(def, supported);
				if (property != NULL)
				{
					inFeature->AddProperty(name, *property);
				}
			}

			return property;
		}

		Value* FeatureMatrix::UpdateValue(Property* inProperty, const std::map<std::string, std::string>& inAttrs)
		{
			if (inProperty == NULL)
			{
				return NULL;
			}

			// name: mandatory attribute
			std::string name;
			std::map<std::string, std::string>::const_iterator itr = inAttrs.find(kAttribute_name);
			if ((itr == inAttrs.end()) || (itr->second.empty()))
			{
				return NULL;
			}
			else
			{
				name = itr->second;
			}

			// supported: optional attribute
			bool supported = true;
			itr = inAttrs.find(kAttribute_supported);
			if (itr != inAttrs.end())
			{
				supported = (itr->second == kValue_true);
			}

			// Find or Create new Value
			Value* pValue = inProperty->FindValue(name);
			if (pValue == NULL)
			{
				pValue = new Value(supported);
				if (pValue != NULL)
				{
					inProperty->AddValue(name, pValue);
				}
			}

			return pValue;
		}
	}
}