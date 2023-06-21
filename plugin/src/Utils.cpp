/*************************************************************************
* ADOBE SYSTEMS INCORPORATED
* Copyright 2013 Adobe Systems Incorporated
* All Rights Reserved.

* NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the
* terms of the Adobe license agreement accompanying it.  If you have received this file from a
* source other than Adobe, then your use, modification, or distribution of it requires the prior
* written permission of Adobe.
**************************************************************************/

#include "Utils.h"

#ifdef _WINDOWS
    #include "Windows.h"
#endif

#ifdef __APPLE__
    #include "CoreFoundation/CoreFoundation.h"
    #include <dlfcn.h>   
    #include <copyfile.h>
#endif

#include <sstream>
#include <codecvt>

#include "IFCMStringUtils.h"

#include <string>
#include <cstring>
#include <stdlib.h>
#include "Application/Service/IOutputConsoleService.h"
#include "Application/Service/IApplicationService.h"
#include "ApplicationFCMPublicIDs.h"

#ifdef _WINDOWS
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#endif

namespace sc {
    namespace Adobe
    {
        static std::string comma = ",";
    }
}


const std::string FixSlashes(const std::string& path) {

#ifdef _WINDOWS
    std::string newString = std::string(path);
    std::replace(newString.begin(), newString.end(), '/', '\\');
    return newString;
#else
    return path;
#endif

}

namespace sc {
    namespace Adobe
    {
        std::u16string Utils::ToUtf16(std::string string) {
            static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
            return convert.from_bytes(string);
        }

        std::string Utils::ToUtf8(std::u16string string) {
            std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
            return convert.to_bytes(string);
        }

        std::string Utils::ToString(const FCM::FCMGUID& in)
        {
            std::ostringstream result;
            unsigned i;

            result.fill('0');

            result << std::hex;
            result << std::setw(8) << (in.Data1);
            result << "-";
            result << std::setw(4) << (in.Data2);
            result << "-";
            result << std::setw(4) << (in.Data3);
            result << "-";

            for (i = 0; i < 2; ++i)
            {
                result << std::setw(2) << (unsigned int)(in.Data4[i]);
            }
            result << "-";

            for (; i < 8; ++i)
            {
                result << std::setw(2) << (unsigned int)(in.Data4[i]);
            }

            std::string guid_str = result.str();

            std::transform(guid_str.begin(), guid_str.end(), guid_str.begin(), ::toupper);

            return guid_str;
        }

        std::string Utils::ToString(const double& in)
        {
            char buffer[32];
            sprintf(buffer, "%.6f", in);
            std::string str(buffer);
            return str;
        }

        std::string Utils::ToString(const float& in)
        {
            char buffer[32];
            sprintf(buffer, "%.6f", in);
            std::string str(buffer);
            return str;
        }

        std::string Utils::ToString(const FCM::U_Int32& in)
        {
            char buffer[32];
            sprintf(buffer, "%u", in);
            std::string str(buffer);
            return str;
        }

        std::string Utils::ToString(const FCM::S_Int32& in)
        {
            char buffer[32];
            sprintf(buffer, "%d", in);
            std::string str(buffer);
            return str;
        }

        std::string Utils::ToString(const DOM::Utils::MATRIX2D& matrix)
        {
            std::string matrixString = "";

            matrixString.append(ToString(matrix.a));
            matrixString.append(comma);
            matrixString.append(ToString(matrix.b));
            matrixString.append(comma);
            matrixString.append(ToString(matrix.c));
            matrixString.append(comma);
            matrixString.append(ToString(matrix.d));
            matrixString.append(comma);
            matrixString.append(ToString(matrix.tx));
            matrixString.append(comma);
            matrixString.append(ToString(matrix.ty));

            return matrixString;
        }

        std::string Utils::ToString(const DOM::Utils::CapType& capType)
        {
            std::string str;

            switch (capType)
            {
            case DOM::Utils::NO_CAP:
                str = "butt";
                break;

            case DOM::Utils::ROUND_CAP:
                str = "round";
                break;

            case DOM::Utils::SQUARE_CAP:
                str = "square";
                break;
            }

            return str;
        }

        std::string Utils::ToString(const DOM::Utils::JoinType& joinType)
        {
            std::string str;

            switch (joinType)
            {
            case DOM::Utils::MITER_JOIN:
                str = "miter";
                break;

            case DOM::Utils::ROUND_JOIN:
                str = "round";
                break;

            case DOM::Utils::BEVEL_JOIN:
                str = "bevel";
                break;
            }

            return str;
        }

        std::string Utils::ToString(const DOM::FillStyle::GradientSpread& spread)
        {
            std::string res;

            switch (spread)
            {
            case DOM::FillStyle::GRADIENT_SPREAD_EXTEND:
                res = "pad";
                break;

            case DOM::FillStyle::GRADIENT_SPREAD_REFLECT:
                res = "reflect";
                break;

            case DOM::FillStyle::GRADIENT_SPREAD_REPEAT:
                res = "repeat";
                break;

            default:
                res = "none";
                break;
            }

            return res;
        }


        std::string Utils::ToString(const DOM::Utils::COLOR& color)
        {
            char cstr[5];
            std::string colorStr;

            colorStr.append("#");
            sprintf(cstr, "%02x", color.red);
            colorStr.append(cstr);
            sprintf(cstr, "%02x", color.green);
            colorStr.append(cstr);
            sprintf(cstr, "%02x", color.blue);
            colorStr.append(cstr);

            return colorStr;
        }

        bool Utils::ReadString(
            const FCM::PIFCMDictionary dict,
            FCM::StringRep8 key,
            std::string& result)
        {
            FCM::U_Int32 valueLen;
            FCM::FCMDictRecTypeID type;

            FCM::Result res = dict->GetInfo(key, type, valueLen);
            if (FCM_FAILURE_CODE(res) || type != FCM::FCMDictRecTypeID::kFCMDictType_StringRep8)
            {
                return false;
            }

            FCM::StringRep8 strValue = new char[valueLen];
            res = dict->Get(key, type, (FCM::PVoid)strValue, valueLen);
            if (FCM_FAILURE_CODE(res))
            {
                delete[] strValue;
                return false;
            }

            result = strValue;

            delete[] strValue;
            return true;
        }

        fs::path Utils::CurrentPath() {
#ifdef _WINDOWS
            char16_t* pathPtr = new char16_t[MAX_PATH];
            GetModuleFileName((HINSTANCE)&__ImageBase, (LPWSTR)pathPtr, MAX_PATH - 1);

            fs::path path(std::u16string((const char16_t*)pathPtr));
            delete[] pathPtr;

            return path.parent_path();
#else
#error Not implemented
#endif
        }

        void Utils::TransformPoint(
            const DOM::Utils::MATRIX2D& matrix,
            DOM::Utils::POINT2D& inPoint,
            DOM::Utils::POINT2D& outPoint)
        {
            DOM::Utils::POINT2D loc;

            loc.x = inPoint.x * matrix.a + inPoint.y * matrix.c + matrix.tx;
            loc.y = inPoint.x * matrix.b + inPoint.y * matrix.d + matrix.ty;

            outPoint = loc;
        }
    }
}
