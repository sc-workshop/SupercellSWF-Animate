/*************************************************************************
* ADOBE SYSTEMS INCORPORATED
* Copyright 2013 Adobe Systems Incorporated
* All Rights Reserved.

* NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the
* terms of the Adobe license agreement accompanying it.  If you have received this file from a
* source other than Adobe, then your use, modification, or distribution of it requires the prior
* written permission of Adobe.
**************************************************************************/

/*
 * @file  DocType.h
 *
 * @brief This file contains declarations for a DocType plugin.
 */


#ifndef DOC_TYPE_H_
#define DOC_TYPE_H_

#include <map>

#include "FCMTypes.h"
#include "FCMPluginInterface.h"
#include "ApplicationFCMPublicIDs.h"
#include "DocType/IDocType.h"
#include "DocType/IFeatureMatrix.h"
#include <string>
#include "PluginConfiguration.h"

/* -------------------------------------------------- Forward Decl */

using namespace FCM;
using namespace DocType;

namespace SupercellSWF
{
    class ModuleDocType;
    class FeatureMatrix;
    class Value;
    class Property;
    class Feature;
    class FeatureDocumentHandler;
}

 
/* -------------------------------------------------- Enums */


/* -------------------------------------------------- Macros / Constants */


/* -------------------------------------------------- Structs / Unions */

namespace SupercellSWF
{
    typedef std::map<std::string, Value*> StrValueMap;
    typedef std::map<std::string, Property*> StrPropertyMap;
    typedef std::map<std::string, Feature*> StrFeatureMap;
};


/* -------------------------------------------------- Class Decl */

namespace SupercellSWF
{
    class ModuleDocType : public DocType::IDocType, public FCM::FCMObjectBase
    {
        BEGIN_INTERFACE_MAP(ModuleDocType, PLUGIN_VERSION)
            INTERFACE_ENTRY(IDocType)    
        END_INTERFACE_MAP
            
    public:

        virtual FCM::Result _FCMCALL GetFeatureMatrix(DocType::PIFeatureMatrix& pFeatureMatrix);

        ModuleDocType();

        ~ModuleDocType();

    private:

        DocType::PIFeatureMatrix m_featureMatrix;
    };

    
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

        Property* UpdateProperty(Feature* inFeature, const std::map<std::string,std::string>& inAttrs);

        Value* UpdateValue(Property* inProperty, const std::map<std::string, std::string>& inAttrs);
            
    private:

        StrFeatureMap mFeatures;

        Feature* mCurrentFeature;

        Property* mCurrentProperty;

        bool m_Inited;
        
        friend class FeatureDocumentHandler;
    };


    class Value
    {
    public:

        Value(bool supported);

        ~Value();

        bool IsSupported();

    private:
        bool mbSupported;
    };


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

    FCM::Result RegisterDocType(FCM::PIFCMDictionary pPlugins, const std::string& resPath);
};


#endif // DOC_TYPE_H_

