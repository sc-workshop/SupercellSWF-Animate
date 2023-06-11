#pragma once

#include "FCMTypes.h"
#include "DocType/IDocType.h"
#include "Module/Version.h"
#include "Module/FCM_Identifiers.h"

#include <map>
#include <string>

using namespace FCM;
using namespace DocType;

namespace sc {
	namespace Adobe
	{
		class DocumentType : public DocType::IDocType, public FCMObjectBase
		{
			BEGIN_INTERFACE_MAP(DocumentType, PLUGIN_VERSION)
				INTERFACE_ENTRY(IDocType)
			END_INTERFACE_MAP

		public:

			virtual Result GetFeatureMatrix(DocType::PIFeatureMatrix& pFeatureMatrix);

			DocumentType();

			~DocumentType();

		private:

			DocType::PIFeatureMatrix m_featureMatrix;
		};

		FCM::Result RegisterDocType(FCM::PIFCMDictionary pPlugins);
	}
}