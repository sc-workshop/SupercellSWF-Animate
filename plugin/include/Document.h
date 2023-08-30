#pragma once

#include <string>

#include "FCMTypes.h"
#include "ApplicationFCMPublicIDs.h"
#include "DocType/IDocType.h"

#include "FeatureMatrix/FeatureMatrix.h"
#include "Module/FCMClasses.h"
#include "PluginConfiguration.h"
#include "Module/Version.h"
#include "Utils.h"

using namespace FCM;
using namespace DocType;

namespace sc {
	namespace Adobe {
		class SCDocumentType : public DocType::IDocType, public FCM::FCMObjectBase
		{
			BEGIN_INTERFACE_MAP(SCDocumentType, PLUGIN_VERSION)
				INTERFACE_ENTRY(IDocType)
			END_INTERFACE_MAP

		public:

			virtual FCM::Result _FCMCALL GetFeatureMatrix(DocType::PIFeatureMatrix& pFeatureMatrix);

			SCDocumentType();

			~SCDocumentType();

		private:

			DocType::PIFeatureMatrix m_fm;
		};

		FCM::Result RegisterDocType(FCM::PIFCMDictionary pPlugins);
	}
}