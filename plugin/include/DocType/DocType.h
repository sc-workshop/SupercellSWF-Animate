#pragma once

#include "FCMTypes.h"
#include "DocType/IDocType.h"
#include "Module/Version.h"
#include "Module/FCM_Identifiers.h"

#include <map>
#include <string>

namespace sc {
	namespace Adobe
	{
		class DocumentType : public DocType::IDocType, public FCM::FCMObjectBase
		{
		public:
			// :skull:
			virtual FCM::PIFCMCallback GetCallback() = 0;

			static FCM::U_Int32 GetVersion() { return (FCM::U_Int32)PLUGIN_VERSION; }

			static FCM::FCMInterfaceMap* GetInterfaceMap()
			{
				static FCM::FCMInterfaceMap _pInterfaceMap[] = {
					{
						FCM::IID_IFCMUnknown,
						((FCM::S_Int64)(static_cast<IFCMUnknown*>((DocumentType*)1)) - 1),
						((FCM::_FCM_CREATORARGFUNC*)0)
					},
					{
						DocType::IID_IDocType,
						((FCM::S_Int64)(static_cast<IDocType*>((DocumentType*)1)) - 1),
						((FCM::_FCM_CREATORARGFUNC*)0)
					},
					{
						FCM::FCMIID_NULL,
						0,
						((FCM::_FCM_CREATORARGFUNC*)0)
					 }
				};

				return _pInterfaceMap;
			}

		public:

			virtual FCM::Result GetFeatureMatrix(DocType::PIFeatureMatrix& pFeatureMatrix);

			DocumentType();

			~DocumentType();

		private:

			DocType::PIFeatureMatrix m_featureMatrix;
		};

		FCM::Result RegisterDocType(FCM::PIFCMDictionary pPlugins);
	}
}