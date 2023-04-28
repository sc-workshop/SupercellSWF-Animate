#include "DocType/DataTypes.h"

namespace Adobe {
	/* -------------------------------------------------- Value */

	Value::Value(bool supported)
	{
		m_supported = supported;
	}

	Value::~Value()
	{
	}

	bool Value::IsSupported()
	{
		return m_supported;
	}

	/* -------------------------------------------------- Property */

	Property::Property(const std::string& def, bool supported)
	{
		m_supported = supported;
		m_default = def;
	}

	Property::~Property()
	{
		m_values.clear();
	}

	Value* Property::FindValue(const std::string& inValueName)
	{
		ValueMap::iterator itr = m_values.find(inValueName);
		if (itr != m_values.end())
			return &itr->second;
		return NULL;
	}

	bool Property::AddValue(const std::string& valueName, Value value)
	{
		m_values.insert(std::pair<std::string, Value>(valueName, value));

		return true;
	}

	bool Property::IsSupported()
	{
		return m_supported;
	}

	std::string Property::GetDefault()
	{
		return m_default;
	}

	/* -------------------------------------------------- Feature */

	Feature::Feature(bool supported)
	{
		m_supported = supported;
	}

	Feature::~Feature()
	{
		m_properties.clear();
	}

	Property* Feature::FindProperty(const std::string& inPropertyName)
	{
		PropertyMap::iterator itr = m_properties.find(inPropertyName);
		if (itr != m_properties.end())
		{
			return &itr->second;
		}
		return NULL;
	}

	bool Feature::AddProperty(const std::string& name, Property property)
	{
		m_properties.insert(std::pair<std::string, Property>(name, property));

		return true;
	}

	bool Feature::IsSupported()
	{
		return m_supported;
	}
}