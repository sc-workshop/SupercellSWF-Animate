#include "DocType/DocType.h"

#include <ApplicationFCMPublicIDs.h>

#include "DocType/FeatureMatrix.h"
#include "Utils.h"
#include "Ids.h"


namespace SupercellSWF {
	ModuleDocumentType::ModuleDocumentType()
	{
		m_featureMatrix = NULL;
	}

	ModuleDocumentType::~ModuleDocumentType()
	{
		FCM_RELEASE(m_featureMatrix);
	}

	FCM::Result ModuleDocumentType::GetFeatureMatrix(PIFeatureMatrix& pFeatureMatrix)
	{
		FCM::Result res = FCM_SUCCESS;

		if (m_featureMatrix == NULL)
		{
			res = GetCallback()->CreateInstance(
				NULL,
				CLSID_FeatureMatrix,
				ID_IFEATURE_MATRIX,
				(FCM::PPVoid)&m_featureMatrix);

			((FeatureMatrix*)m_featureMatrix)->Init(GetCallback());
		}

		pFeatureMatrix = m_featureMatrix;
		return res;
	}

	FCM::Result RegisterDocType(FCM::PIFCMDictionary pPlugins, const std::string& resPath)
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

		{
			// Level 1 Dictionary
			AutoPtr<IFCMDictionary> pPlugin;
			res = pPlugins->AddLevel(
				(const FCM::StringRep8)Utils::ToString(CLSID_DocType).c_str(),
				pPlugin.m_Ptr);

			{
				// Level 2 Dictionary
				AutoPtr<IFCMDictionary> pCategory;
				res = pPlugin->AddLevel((const FCM::StringRep8)kApplicationCategoryKey_DocType, pCategory.m_Ptr);

				{
					// Level 3 Dictionary

					// Add short name - Used in the "New Document Dialog" / "Start Page".
					std::string str_name = DOCTYPE_NAME;
					res = pCategory->Add(
						(const FCM::StringRep8)kApplicationCategoryKey_Name,
						kFCMDictType_StringRep8,
						(FCM::PVoid)str_name.c_str(),
						(FCM::U_Int32)str_name.length() + 1);

					// Add universal name - Used to refer to it from JSFL and used in error messages
					std::string str_name_uni = DOCTYPE_UNIVERSAL_NAME;
					res = pCategory->Add(
						(const FCM::StringRep8)kApplicationCategoryKey_UniversalName,
						kFCMDictType_StringRep8,
						(FCM::PVoid)str_name_uni.c_str(),
						(FCM::U_Int32)str_name_uni.length() + 1);

					// Add plugin description - Appears in the "New Document Dialog"
					// Plugin description can be localized depending on the languageCode.
					std::string str_desc = DOCTYPE_DESCRIPTION;

					if (!resPath.empty())
					{
						// Look for the localized string for description
						std::string path = resPath + "res.txt";
						FILE* fp = fopen(path.c_str(), "rb");
						if (fp != NULL)
						{
							long fileSize;
							std::shared_ptr<char> pDesc;
							size_t num;
							fseek(fp, 0, SEEK_END);
							fileSize = ftell(fp);

							pDesc.reset(new char[fileSize]);

							fseek(fp, 0, SEEK_SET);
							num = fread(pDesc.get(), 1, fileSize, fp);

							if (num > 0)
							{
								// Found description
								str_desc = pDesc.get();
								str_desc[fileSize] = 0;
							}

							fclose(fp);
						}
					}

					res = pCategory->Add(
						(const FCM::StringRep8)kApplicationDocTypeKey_Desc,
						kFCMDictType_StringRep8,
						(FCM::PVoid)str_desc.c_str(),
						(FCM::U_Int32)str_desc.length() + 1);
				}
			}
		}

		return res;
	}
}