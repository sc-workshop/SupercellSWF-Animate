#include "DocType/DocType.h"

#include <ApplicationFCMPublicIDs.h>

#include "DocType/FeatureMatrix.h"
#include "Utils.h"

namespace sc {
	namespace Adobe {
		DocumentType::DocumentType()
		{
			m_featureMatrix = NULL;
		}

		DocumentType::~DocumentType()
		{
			FCM_RELEASE(m_featureMatrix);
		}

		Result DocumentType::GetFeatureMatrix(PIFeatureMatrix& matrix)
		{
			if (m_featureMatrix == NULL)
			{
				GetCallback()->CreateInstance(
					NULL,
					CLSID_FeatureMatrix,
					ID_IFEATURE_MATRIX,
					(FCM::PPVoid)&m_featureMatrix);

				((FeatureMatrix*)m_featureMatrix)->Init(GetCallback());
			}
			else {
				((FeatureMatrix*)m_featureMatrix)->UpdateContext(GetCallback());
			}

			matrix = m_featureMatrix;
			return FCM_SUCCESS;
		}

		FCM::Result RegisterDocType(FCM::PIFCMDictionary pPlugins)
		{
			FCM::Result res = FCM_SUCCESS;

			/*
			 * Dictionary structure for a DocType plugin is as follows:
			 *
			 *  Level 0 :
			 *              --------------------------------
			 *             | Application.Component |  ----- | -----------------------------
			 *              --------------------------------                               |
			 *                                                                             |
			 *  Level 1:                                   <-------------------------------
			 *              -----------------------------
			 *             | CLSID_DocType_GUID |  ----- | --------------------------------
			 *              -----------------------------                                  |
			 *                                                                             |
			 *  Level 2:                                                <------------------
			 *              -------------------------------------------------
			 *             | Application.Component.Category.DocType |  ----- |-------------
			 *              -------------------------------------------------              |
			 *                                                                             |
			 *  Level 3:                                                     <-------------
			 *              -----------------------------------------------------------------------
			 *             | Application.Component.Category.Name          |  DOCTYPE_NAME          |
			 *              -----------------------------------------------------------------------
			 *             | Application.Component.Category.UniversalName |  DOCTYPE_UNIVERSAL_NAME|
			 *              -----------------------------------------------------------------------
			 *             | Application.Component.DocType.Desc           |  DOCTYPE_DESCRIPTION   |
			 *              -----------------------------------------------------------------------
			 *
			 *  Note that before calling this function the level 0 dictionary has already
			 *  been added. Here, the 1st, 2nd and 3rd level dictionaries are being added.
			 */

			 // Level 1 Dictionary
			AutoPtr<IFCMDictionary> pPlugin;
			res = pPlugins->AddLevel(
				(const FCM::StringRep8)Utils::ToString(CLSID_DocType).c_str(),
				pPlugin.m_Ptr
			);

			// Level 2 Dictionary
			AutoPtr<IFCMDictionary> pCategory;
			res = pPlugin->AddLevel((const FCM::StringRep8)kApplicationCategoryKey_DocType, pCategory.m_Ptr);

			// Level 3 Dictionary

			// Add short name - Used in the "New Document Dialog" / "Start Page".
			std::string documentName = DOCTYPE_NAME;
			res = pCategory->Add(
				(const FCM::StringRep8)kApplicationCategoryKey_Name,
				kFCMDictType_StringRep8,
				(FCM::PVoid)documentName.c_str(),
				(FCM::U_Int32)documentName.length() + 1
			);

			// Add universal name - Used to refer to it from JSFL and used in error messages
			std::string documentUniversalName = DOCTYPE_UNIVERSAL_NAME;
			res = pCategory->Add(
				(const FCM::StringRep8)kApplicationCategoryKey_UniversalName,
				kFCMDictType_StringRep8,
				(FCM::PVoid)documentUniversalName.c_str(),
				(FCM::U_Int32)documentUniversalName.length() + 1
			);

			// Add plugin description - Appears in the "New Document Dialog"
			std::string documentDescription = "";

			res = pCategory->Add(
				(const FCM::StringRep8)kApplicationDocTypeKey_Desc,
				kFCMDictType_StringRep8,
				(FCM::PVoid)documentDescription.c_str(),
				(FCM::U_Int32)documentDescription.length() + 1
			);

			return res;
		}
	}
}