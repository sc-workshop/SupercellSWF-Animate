#pragma once

#include "FCMTypes.h"
#include "DocType/IDocType.h"
#include "PluginConfiguration.h"

#include <map>
#include <string>

using namespace FCM;
using namespace DocType;

namespace SupercellSWF
{
	class ModuleDocumentType : public DocType::IDocType, public FCMObjectBase
	{
		BEGIN_INTERFACE_MAP(ModuleDocumentType, PLUGIN_VERSION)
			INTERFACE_ENTRY(IDocType)
		END_INTERFACE_MAP

	public:

		virtual Result _FCMCALL GetFeatureMatrix(DocType::PIFeatureMatrix& pFeatureMatrix);

		ModuleDocumentType();

		~ModuleDocumentType();

	private:

		DocType::PIFeatureMatrix m_featureMatrix;
	};

	FCM::Result RegisterDocType(FCM::PIFCMDictionary pPlugins, const std::string& resPath);
}