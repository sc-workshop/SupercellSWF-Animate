#include "DocType/FeatureMatrix.h"

#include "PluginConfiguration.h"
#include "Utils.h"

#include <Application/Service/IOutputConsoleService.h>
#include <FlashFCMPublicIDs.h>

#include <fstream>
#include <sstream>

#include <filesystem>

namespace fs = std::filesystem;

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
		m_inited = true;
	}

	FeatureMatrix::~FeatureMatrix()
	{
	}

	void FeatureMatrix::Init(FCM::PIFCMCallback pCallback)
	{
		if (m_inited)
		{
			return;
		}

		std::string modulePath;
		Utils::GetModuleFilePath(modulePath, pCallback);

		fs::path featuresPath(modulePath);
		featuresPath /= "res";
		featuresPath /= "Features.xml";

		// trace
		FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
		FCM::Result res = pCallback->GetService(Application::Service::APP_OUTPUT_CONSOLE_SERVICE, pUnk.m_Ptr);
		ASSERT(FCM_SUCCESS_CODE(res));

		FCM::AutoPtr<Application::Service::IOutputConsoleService> outputConsoleService = pUnk;
		FCM::StringRep16 path = Utils::ToString16(featuresPath.string(), pCallback);

		FCM::AutoPtr<FCM::IFCMCalloc> pCalloc = Utils::GetCallocService(pCallback);
		ASSERT(pCalloc.m_Ptr != NULL);

		pCalloc->Free(path);
		path = NULL;

		std::ifstream xmlFile(featuresPath, std::ifstream::in | std::ifstream::binary);

		char* buffer = NULL;
		long length = 0;

		if (xmlFile)
		{
			xmlFile.seekg(0, xmlFile.end);
			length = (long)xmlFile.tellg();
			xmlFile.seekg(0, xmlFile.beg);
			buffer = new char[length + 1];
			xmlFile.read(buffer, length);
			buffer[length] = 0;
		}
		xmlFile.close();

		delete[] buffer;
	}

	FCM::Result FeatureMatrix::IsSupported(CStringRep16 inFeatureName, FCM::Boolean& isSupported)
	{
		std::string featureLC = Utils::ToString(inFeatureName, GetCallback());

		Feature* pFeature = FindFeature(featureLC);
		if (pFeature == NULL)
		{
			/* If a feature is not found, it is supported */
			isSupported = true;
		}
		else
		{
			isSupported = pFeature->IsSupported();
		}
		return FCM_SUCCESS;
	}

	FCM::Result FeatureMatrix::IsSupported(
		CStringRep16 inFeatureName,
		CStringRep16 inPropName,
		FCM::Boolean& isSupported)
	{
		std::string featureLC = Utils::ToString(inFeatureName, GetCallback());

		Feature* pFeature = FindFeature(featureLC);
		if (pFeature == NULL)
		{
			/* If a feature is not found, it is supported */
			isSupported = true;
		}
		else
		{
			if (!pFeature->IsSupported())
			{
				/* If a feature is not supported, sub-features are not supported */
				isSupported = false;
			}
			else
			{
				// Look if sub-features are supported.
				std::string propertyLC = Utils::ToString(inPropName, GetCallback());

				Property* pProperty = pFeature->FindProperty(propertyLC);
				if (pProperty == NULL)
				{
					/* If a property is not found, it is supported */
					isSupported = true;
				}
				else
				{
					isSupported = pProperty->IsSupported();
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
		std::string featureLC = Utils::ToString(inFeatureName, GetCallback());

		Feature* pFeature = FindFeature(featureLC);
		if (pFeature == NULL)
		{
			/* If a feature is not found, it is supported */
			isSupported = true;
		}
		else
		{
			if (!pFeature->IsSupported())
			{
				/* If a feature is not supported, sub-features are not supported */
				isSupported = false;
			}
			else
			{
				std::string propertyLC(Utils::ToString(inPropName, GetCallback()));

				Property* pProperty = pFeature->FindProperty(propertyLC);
				if (pProperty == NULL)
				{
					/* If a property is not found, it is supported */
					isSupported = true;
				}
				else
				{
					if (!pProperty->IsSupported())
					{
						/* If a property is not supported, all values are not supported */
						isSupported = false;
					}
					else
					{
						std::string valueLC(Utils::ToString(inValName, GetCallback()));

						Value* pValue = pProperty->FindValue(valueLC);
						if (pValue == NULL)
						{
							/* If a value is not found, it is supported */
							isSupported = true;
						}
						else
						{
							isSupported = pValue->IsSupported();
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
		std::string featureName = Utils::ToString(inFeatureName, GetCallback());
		std::string propName = Utils::ToString(inPropName, GetCallback());

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
				case kFCMVarype_CString: outDefVal.m_value.strVal = Utils::ToString16(strVal, GetCallback()); break;
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
		StrFeatureMap::iterator itr = m_features.find(inFeatureName);
		if (itr != m_features.end())
		{
			return itr->second;
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
		Property* pProperty = NULL;
		pProperty = inFeature->FindProperty(name);
		if (pProperty == NULL)
		{
			pProperty = new Property(def, supported);
			if (pProperty != NULL)
			{
				inFeature->AddProperty(name, pProperty);
			}
		}

		return pProperty;
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