#pragma once

#include "FCMTypes.h"
#include "FCMPluginInterface.h"
#include "Utils/DOMTypes.h"
#include <string>
#include "FillStyle/IGradientFillStyle.h"
#include "IFCMStringUtils.h"
#include <iostream>
#include <fstream>

#include <filesystem>
namespace fs = std::filesystem;

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

namespace sc {
    namespace Adobe {
        class Utils
        {
        public:
            static std::u16string ToUtf16(std::string string);

            static std::string ToUtf8(std::u16string string);

            static std::string ToString(const FCM::FCMGUID& in);

            static std::string ToString(const double& in);

            static std::string ToString(const float& in);

            static std::string ToString(const FCM::U_Int32& in);

            static std::string ToString(const FCM::S_Int32& in);

            static std::string ToString(const DOM::Utils::MATRIX2D& matrix);

            static std::string ToString(const DOM::Utils::CapType& capType);

            static std::string ToString(const DOM::Utils::JoinType& joinType);

            static std::string ToString(const DOM::FillStyle::GradientSpread& spread);

            static std::string ToString(const DOM::Utils::COLOR& color);

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
                    delete value;
                    return false;
                }

                result = *value;

                delete value;
                return true;
            }

            static fs::path CurrentPath();

            static void TransformPoint(
                const DOM::Utils::MATRIX2D& matrix,
                DOM::Utils::POINT2D& inPoint,
                DOM::Utils::POINT2D& outPoint);
        };
    };
};
