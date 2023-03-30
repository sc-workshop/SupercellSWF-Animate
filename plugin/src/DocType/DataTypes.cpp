#include "DocType/DataTypes.h"

namespace SupercellSWF {
	/* -------------------------------------------------- Value */

	Value::Value(bool supported)
	{
		mbSupported = supported;
	}

	Value::~Value()
	{
	}

	bool Value::IsSupported()
	{
		return mbSupported;
	}

	/* -------------------------------------------------- Property */

	Property::Property(const std::string& def, bool supported)
	{
		mbSupported = supported;
		mDefault = def;
	}

	Property::~Property()
	{
		StrValueMap::iterator itr = mValues.begin();
		for (; itr != mValues.end(); itr++)
		{
			if (itr->second) delete itr->second;
		}
		mValues.clear();
	}

	Value* Property::FindValue(const std::string& inValueName)
	{
		StrValueMap::iterator itr = mValues.find(inValueName);
		if (itr != mValues.end())
			return itr->second;
		return NULL;
	}

	bool Property::AddValue(const std::string& valueName, Value* pValue)
	{
		mValues.insert(std::pair<std::string, Value*>(valueName, pValue));

		return true;
	}

	bool Property::IsSupported()
	{
		return mbSupported;
	}

	std::string Property::GetDefault()
	{
		return mDefault;
	}

	/* -------------------------------------------------- Feature */

	Feature::Feature(bool supported)
	{
		mbSupported = supported;
	}

	Feature::~Feature()
	{
		StrPropertyMap::iterator itr = mProperties.begin();
		for (; itr != mProperties.end(); itr++)
		{
			if (itr->second) delete itr->second;
		}
		mProperties.clear();
	}

	Property* Feature::FindProperty(const std::string& inPropertyName)
	{
		StrPropertyMap::iterator itr = mProperties.find(inPropertyName);
		if (itr != mProperties.end())
		{
			return itr->second;
		}
		return NULL;
	}

	bool Feature::AddProperty(const std::string& name, Property* pProperty)
	{
		mProperties.insert(std::pair<std::string, Property*>(name, pProperty));

		return true;
	}

	bool Feature::IsSupported()
	{
		return mbSupported;
	}
}