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
		bool mbSupported;
	};

	typedef std::map<std::string, Value*> StrValueMap;

	class Property
	{
	public:
		Property(const std::string& def, bool supported);

		~Property();

		Value* FindValue(const std::string& inValueName);

		bool AddValue(const std::string& valueName, Value* pValue);

		bool IsSupported();

		std::string GetDefault();

	private:
		std::string mDefault;
		bool mbSupported;
		StrValueMap mValues;
	};

	typedef std::map<std::string, Property*> StrPropertyMap;

	class Feature
	{
	public:

		Feature(bool supported);

		~Feature();

		Property* FindProperty(const std::string& inPropertyName);

		bool AddProperty(const std::string& name, Property* pProperty);

		bool IsSupported();

	private:

		bool mbSupported;

		StrPropertyMap mProperties;
	};

	typedef std::map<std::string, Feature*> StrFeatureMap;
}