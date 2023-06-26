#pragma once

#include <string>
#include <map>

namespace sc {
	namespace Adobe {
        class Value
        {
        public:

            Value(bool supported);

            ~Value();

            bool IsSupported();

        private:
            bool m_isSupported;
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
            bool m_isSupported;
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
            bool m_isSupported;
            PropertyMap m_properties;
        };

        typedef std::map<std::string, Feature> FeatureMap;
	}
}