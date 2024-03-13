#pragma once

#include <string>

#include "AnimateSDK/core/common/FCMTypes.h"
#include "AnimateSDK/app/ApplicationFCMPublicIDs.h"
#include "AnimateSDK/app/DocType/IDocType.h"

#include "FeatureMatrix/FeatureMatrix.h"
#include "Module/FCMClasses.h"
#include "PluginConfiguration.h"
#include "Module/Version.h"

using namespace FCM;
using namespace DocType;

namespace sc {
	namespace Adobe {
		class GenericDocumentType : public DocType::IDocType, public FCM::FCMObjectBase
		{
			BEGIN_INTERFACE_MAP(GenericDocumentType, PLUGIN_VERSION)
				INTERFACE_ENTRY(IDocType)
			END_INTERFACE_MAP

		public:
			virtual FCM::Result _FCMCALL GetFeatureMatrix(DocType::PIFeatureMatrix& pFeatureMatrix);

			GenericDocumentType();

			virtual ~GenericDocumentType();

		private:

			FeatureMatrix* m_features;
		};

		FCM::Result RegisterDocType(FCM::PIFCMDictionary pPlugins);
	}
}