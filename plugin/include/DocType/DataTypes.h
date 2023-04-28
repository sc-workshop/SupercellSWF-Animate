#pragma once

#include <map>
#include <string>

namespace Adobe {
	class Value
	{
	public:

		Value(bool supported);

		~Value();

		bool IsSupported();

	private:
		bool m_supported;
	};

	typedef std::map<std::string, Value> ValueMap;

	class Property
	{
	public:
		Property(const std::string& def, bool supported);

		~Property();

		Value* FindValue(const std::string& inValueName);

		bool AddValue(const std::string& valueName, Value value);

		bool IsSupported();

		std::string GetDefault();

	private:
		std::string m_default;
		bool m_supported;
		ValueMap m_values;
	};

	typedef std::map<std::string, Property> PropertyMap;

	class Feature
	{
	public:

		Feature(bool supported);

		~Feature();

		Property* FindProperty(const std::string& inPropertyName);

		bool AddProperty(const std::string& name, Property property);

		bool IsSupported();

	private:

		bool m_supported;

		PropertyMap m_properties;
	};

	typedef std::map<std::string, Feature> FeatureMap;
}