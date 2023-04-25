#pragma once

#include "FCMTypes.h"
#include "FCMPluginInterface.h"
#include "Utils/DOMTypes.h"
#include <string>
#include "FillStyle/IGradientFillStyle.h"
#include "IFCMStringUtils.h"
#include <iostream>
#include <fstream>

/* -------------------------------------------------- Forward Decl */


/* -------------------------------------------------- Enums */


/* -------------------------------------------------- Macros / Constants */

#ifdef _DEBUG
    #define ASSERT(cond)                    \
    {                                       \
        if (!(cond))                        \
        {                                   \
            assert(0);                      \
        }                                   \
    }

#else
    #define ASSERT(cond)

#endif


/* -------------------------------------------------- Class Decl */

namespace Adobe {
    class Utils
    {
    public:

        static std::string ToString(const FCM::FCMGUID& in);

        static std::string ToString(const double& in);

        static std::string ToString(const float& in);

        static std::string ToString(const FCM::U_Int32& in);

        static std::string ToString(const FCM::S_Int32& in);

        static std::string ToString(const DOM::Utils::MATRIX2D& matrix);

        static std::string ToString(const DOM::Utils::CapType& capType);

        static std::string ToString(const DOM::Utils::JoinType& joinType);

        static std::string ToString(FCM::CStringRep16 pStr16, FCM::PIFCMCallback pCallback);

        static std::string ToString(FCM::CStringRep8 pStr8);

        static FCM::StringRep16 ToString16(const std::string& str, FCM::PIFCMCallback pCallback);

        static std::string ToString(const DOM::FillStyle::GradientSpread& spread);

        static std::string ToString(const DOM::Utils::COLOR& color);

        static std::string ToString(const DOM::Utils::COLOR_MATRIX& color);

        static bool ReadString(
            const FCM::PIFCMDictionary pDict,
            FCM::StringRep8 key,
            std::string& retString);

        static bool ReadGUID(
            const FCM::PIFCMDictionary pDict,
            FCM::StringRep8 key,
            FCM::FCMGUID& result) {

            FCM::U_Int32 valueLen;
            FCM::FCMDictRecTypeID type;

            FCM::Result res = pDict->GetInfo(key, type, valueLen);
            if (FCM_FAILURE_CODE(res))
            {
                return false;
            }

            FCM::FCMGUID* value = new FCM::FCMGUID();
            res = pDict->Get(key, type, (FCM::PVoid)value, valueLen);
            if (FCM_FAILURE_CODE(res))
            {
                delete[] value;
                return false;
            }

            result = *value;

            delete[] value;
            return true;
        }

        static void TransformPoint(
            const DOM::Utils::MATRIX2D& matrix,
            DOM::Utils::POINT2D& inPoint,
            DOM::Utils::POINT2D& outPoint);

        static void GetParent(const std::string& path, std::string& parent);

        static void GetFileName(const std::string& path, std::string& fileName);

        static void GetFileNameWithoutExtension(const std::string& path, std::string& fileName);

        static void GetFileExtension(const std::string& path, std::string& extension);

        static void GetModuleFilePath(std::string& path, FCM::PIFCMCallback pCallback);

        static FCM::Result CreateDir(const std::string& path, FCM::PIFCMCallback pCallback);

        static FCM::AutoPtr<FCM::IFCMCalloc> GetCallocService(FCM::PIFCMCallback pCallback);

        static FCM::AutoPtr<FCM::IFCMStringUtils> GetStringUtilsService(FCM::PIFCMCallback pCallback);

        static void GetLanguageCode(FCM::PIFCMCallback pCallback, std::string& langCode);

        static void GetAppVersion(FCM::PIFCMCallback pCallback, FCM::U_Int32& version);

        static void OpenFStream(const std::string& outputFileName, std::fstream& file, std::ios_base::openmode mode, FCM::PIFCMCallback pCallback);

        static FCM::Result CopyDir(const std::string& srcFolder, const std::string& dstFolder, FCM::PIFCMCallback pCallback);

        static FCM::Result Remove(const std::string& folder, FCM::PIFCMCallback pCallback);
    };
};
