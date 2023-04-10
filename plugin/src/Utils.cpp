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
    #include "ShellApi.h"
#endif

#ifdef __APPLE__
    #include "CoreFoundation/CoreFoundation.h"
    #include <dlfcn.h>   
    #include <copyfile.h>
#endif

#include <iomanip>
#include <algorithm>
#include <sstream>

#include "IFCMStringUtils.h"

#include <string>
#include <cstring>
#include <stdlib.h>
#include "Application/Service/IOutputConsoleService.h"
#include "Application/Service/IApplicationService.h"
#include "ApplicationFCMPublicIDs.h"

/* -------------------------------------------------- Constants */

#ifdef _WINDOWS
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#endif

namespace SupercellSWF
{
    static std::string comma = ",";
}


/* -------------------------------------------------- Static Functions */

const std::string FixSlashes(const std::string& path) {

#ifdef _WINDOWS
    std::string newString = std::string(path);
    std::replace(newString.begin(), newString.end(), '/', '\\');
    return newString;
#else
    return path;
#endif

}


/* -------------------------------------------------- Utils */

namespace SupercellSWF
{
    FCM::AutoPtr<FCM::IFCMStringUtils> Utils::GetStringUtilsService(FCM::PIFCMCallback pCallback)
    {
        FCM::AutoPtr<FCM::IFCMUnknown> pIFCMStringUtilsUnknown;
        FCM::Result res = pCallback->GetService(FCM::SRVCID_Core_StringUtils, pIFCMStringUtilsUnknown.m_Ptr);
        if (FCM_FAILURE_CODE(res))
        {
            return NULL;
        }
        FCM::AutoPtr<FCM::IFCMStringUtils> pIFCMStringUtils = pIFCMStringUtilsUnknown;
        return pIFCMStringUtils;
    }
    

    FCM::AutoPtr<FCM::IFCMCalloc> Utils::GetCallocService(FCM::PIFCMCallback pCallback)
    {
        FCM::AutoPtr<FCM::IFCMUnknown> pIFCMCallocUnknown;
        FCM::Result res = pCallback->GetService(FCM::SRVCID_Core_Memory, pIFCMCallocUnknown.m_Ptr);
        if (FCM_FAILURE_CODE(res))
        {
            return NULL;
        }
        FCM::AutoPtr<FCM::IFCMCalloc> pIFCMCalloc = pIFCMCallocUnknown;
        return pIFCMCalloc;
    }
    

    void Utils::GetLanguageCode(FCM::PIFCMCallback pCallback, std::string& langCode)
    {
        FCM::StringRep8 pLanguageCode;
        FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
        FCM::AutoPtr<Application::Service::IApplicationService> pAppService;
        FCM::Result res;
        
        res = pCallback->GetService(Application::Service::APP_SERVICE, pUnk.m_Ptr);
        pAppService = pUnk;

        if (pAppService)
        {
            res = pAppService->GetLanguageCode(&pLanguageCode);
            if (FCM_SUCCESS_CODE(res))
            {
                langCode = ToString(pLanguageCode);

                FCM::AutoPtr<FCM::IFCMCalloc> pCalloc = GetCallocService(pCallback);
                pCalloc->Free(pLanguageCode);
            }
        }
    }

    void Utils::GetAppVersion(FCM::PIFCMCallback pCallback, FCM::U_Int32& version)
    {
        FCM::AutoPtr<FCM::IFCMUnknown> pUnk;
        FCM::AutoPtr<Application::Service::IApplicationService> pAppService;
        FCM::Result res;
        
        version = 0;

        res = pCallback->GetService(Application::Service::APP_SERVICE, pUnk.m_Ptr);
        pAppService = pUnk;

        if (pAppService)
        {
            res = pAppService->GetVersion(version);
            ASSERT(FCM_SUCCESS_CODE(res))
        }
    }

    std::string Utils::ToString(const FCM::FCMGUID& in)
    {
        std::ostringstream result;
        unsigned i;

        result.fill('0');

        result << std::hex;
        result  << std::setw(8) << (in.Data1);
        result << "-";
        result  << std::setw(4) << (in.Data2);
        result << "-";
        result  << std::setw(4) << (in.Data3);
        result << "-";

        for (i = 0; i < 2 ; ++i)
        {
            result << std::setw(2) << (unsigned int) (in.Data4[i]);
        }
        result << "-";

        for (; i < 8 ; ++i)
        {
            result << std::setw(2) << (unsigned int) (in.Data4[i]);
        }

        std::string guid_str = result.str();

        std::transform(guid_str.begin(), guid_str.end(), guid_str.begin(), ::toupper);

        return guid_str;
    }

    
    std::string Utils::ToString(FCM::CStringRep16 pStr16, FCM::PIFCMCallback pCallback)
    {
        FCM::StringRep8 pStr8 = NULL;
        FCM::AutoPtr<FCM::IFCMStringUtils> pStrUtils = GetStringUtilsService(pCallback);
        pStrUtils->ConvertStringRep16to8(pStr16, pStr8);
        
        std::string string = (const char*)pStr8;
        
        FCM::AutoPtr<FCM::IFCMCalloc> pCalloc = GetCallocService(pCallback);
        pCalloc->Free(pStr8);
        
        return string;
    }

    std::string Utils::ToString(FCM::CStringRep8 pStr8)
    {
        std::string string = (const char*)pStr8;
        return string;
    }
    
    std::string Utils::ToString(const double& in)
    {
        char buffer[32];
        sprintf(buffer,"%.6f", in);
        std::string str(buffer);
        return str;
    }
    
    std::string Utils::ToString(const float& in)
    {
        char buffer[32];
        sprintf(buffer,"%.6f", in);
        std::string str(buffer);
        return str;
    }
    
    std::string Utils::ToString(const FCM::U_Int32& in)
    {
        char buffer[32];
        sprintf(buffer,"%u", in);
        std::string str(buffer);
        return str;
    }
    
    std::string Utils::ToString(const FCM::S_Int32& in)
    {
        char buffer[32];
        sprintf(buffer,"%d", in);
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

    
    FCM::StringRep16 Utils::ToString16(const std::string& str, FCM::PIFCMCallback pCallback)
    {
        FCM::StringRep16 pStrFeatureName = NULL;
        FCM::AutoPtr<FCM::IFCMStringUtils> pStrUtils = GetStringUtilsService(pCallback);
        pStrUtils->ConvertStringRep8to16(str.c_str(), pStrFeatureName);
        
        return pStrFeatureName;
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
        const FCM::PIFCMDictionary pDict,
        FCM::StringRep8 key,
        std::string& retString)
    {
        FCM::U_Int32 valueLen;
        FCM::FCMDictRecTypeID type;

        FCM::Result res = pDict->GetInfo(key, type, valueLen);
        if (FCM_FAILURE_CODE(res))
        {
            return false;
        }

        FCM::StringRep8 strValue = new char[valueLen];
        res = pDict->Get(key, type, (FCM::PVoid)strValue, valueLen);
        if (FCM_FAILURE_CODE(res))
        {
            delete[] strValue;
            return false;
        }

        retString = strValue;

        delete[] strValue;
        return true;
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

    void Utils::GetParent(const std::string& path, std::string& parent)
    {
        size_t index = path.find_last_of("/\\");
        if((index+1) == path.length())
        {
            parent = path.substr(0, index);
            index = parent.find_last_of("/\\");
        }
        parent = path.substr(0, index + 1);
    }

    void Utils::GetFileName(const std::string& path, std::string& fileName)
    {
        size_t index = path.find_last_of("/\\");
        fileName = path.substr(index + 1, path.length() - index - 1);
    }

    void Utils::GetFileNameWithoutExtension(const std::string& path, std::string& fileName)
    {
        GetFileName(path, fileName);

        // Remove the extension (if any)
        size_t index = fileName.find_last_of(".");
        if (index != std::string::npos)
        {
            fileName = fileName.substr(0, index);
        }
    }

    void Utils::GetFileExtension(const std::string& path, std::string& extension)
    {
        size_t index = path.find_last_of(".");
        extension = "";
        if (index != std::string::npos)
        {
            extension = path.substr(index + 1, path.length());
        }
    }

    void Utils::GetModuleFilePath(std::string& path, FCM::PIFCMCallback pCallback)
    {
#ifdef _WINDOWS

        std::string fullPath;
        FCM::U_Int16* pFilePath = new FCM::U_Int16[MAX_PATH];

        ASSERT(pFilePath);

        ::GetModuleFileName((HINSTANCE)&__ImageBase, (LPWSTR)pFilePath, MAX_PATH - 1);

        fullPath = Utils::ToString(pFilePath, pCallback);

        GetParent(fullPath, path);

        delete[] pFilePath;

#else
        Dl_info info;
        if (dladdr((void*)(GetModuleFilePath), &info)) {
            std::string fullPath(info.dli_fname);
            GetParent(fullPath, path);
            GetParent(path, fullPath);
            GetParent(fullPath, path);
            GetParent(path, fullPath);
            path = fullPath;
        }
        else {
            ASSERT(0);
        }
#endif
    }


    // Creates a directory. If the directory already exists or is successfully created, success
    // is returned; otherwise an error code is returned.
    FCM::Result Utils::CreateDir(const std::string& path, FCM::PIFCMCallback pCallback) {
#ifdef _WINDOWS

        FCM::Result res = FCM_SUCCESS;
        BOOL ret;
        FCM::StringRep16 pFullPath;

        pFullPath = Utils::ToString16(FixSlashes(path), pCallback);
        ASSERT(pFullPath);

        ret = ::CreateDirectory((LPCWSTR)pFullPath, NULL);
        if (ret == FALSE)
        {
            DWORD err = GetLastError();
            if (err != ERROR_ALREADY_EXISTS)
            {
                res = FCM_GENERAL_ERROR;
            }
        }

        FCM::AutoPtr<FCM::IFCMCalloc> pCalloc = Utils::GetCallocService(pCallback);
        ASSERT(pCalloc.m_Ptr != NULL);
        pCalloc->Free(pFullPath);

        return res;

#else
        struct stat sb;

        // Does the directory exist?
        if (stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
        {
            return FCM_SUCCESS;
        }

        int err = mkdir(path.c_str(), 0777);
        if ((err == 0) || (err == EEXIST))
        {

            return FCM_SUCCESS;
        }

        return FCM_GENERAL_ERROR;
#endif
    }

    void Utils::OpenFStream(const std::string& outputFileName, std::fstream &file, std::ios_base::openmode mode, FCM::PIFCMCallback pCallback)
    {
 
#ifdef _WINDOWS
        FCM::StringRep16 pFilePath = Utils::ToString16(outputFileName, pCallback);

        file.open(pFilePath,mode);

        FCM::AutoPtr<FCM::IFCMCalloc> pCalloc = Utils::GetCallocService(pCallback);
        ASSERT(pCalloc.m_Ptr != NULL);  
        pCalloc->Free(pFilePath);
#else
       file.open(outputFileName.c_str(),mode);
#endif
    };

    // Removes the folder all its contents
    FCM::Result Utils::Remove(const std::string& folder, FCM::PIFCMCallback pCallback)
    {

#ifdef _WINDOWS

        SHFILEOPSTRUCT sf;
        std::wstring wstr;

        memset(&sf, 0, sizeof(sf));

        sf.hwnd = NULL;
        sf.wFunc = FO_DELETE;
        sf.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI | FOF_SILENT;
        FCM::StringRep16 folderStr = Utils::ToString16(FixSlashes(folder), pCallback);;
        wstr = std::wstring((const wchar_t*)folderStr);;
        wstr.append(2, '\0');
        sf.pFrom = wstr.c_str();
        sf.pTo = NULL;

        FCM::AutoPtr<FCM::IFCMCalloc> pCalloc = GetCallocService(pCallback);
        int n = SHFileOperation(&sf);
        if (n != 0)
        {
            pCalloc->Free(folderStr);
            return FCM_GENERAL_ERROR;
        }

        pCalloc->Free(folderStr);

#else
        std::string delFolder = folder + "/";
        std::string cmd = "rm -rf " + delFolder;
        std::system(cmd.c_str());
#endif

        return FCM_SUCCESS;
    }


    // Copies a source folder to a destination folder. In other words, dstFolder contains
    // the srcFolder after the operation.
    FCM::Result Utils::CopyDir(const std::string& srcFolder, const std::string& dstFolder, FCM::PIFCMCallback pCallback)
    {
#ifdef _WINDOWS

        SHFILEOPSTRUCT sf;
        std::wstring srcWstr;
        std::wstring dstWstr;

        memset(&sf, 0, sizeof(sf));

        sf.hwnd = NULL;
        sf.wFunc = FO_COPY;
        sf.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI | FOF_SILENT;

        FCM::StringRep16 srcFolderStr = Utils::ToString16(FixSlashes(srcFolder), pCallback);
        srcWstr = std::wstring((const wchar_t*)srcFolderStr);
        srcWstr.append(2, '\0');
        sf.pFrom = srcWstr.c_str();

        FCM::StringRep16 dstFolderStr = Utils::ToString16(FixSlashes(dstFolder), pCallback);
        dstWstr = std::wstring((const wchar_t*)dstFolderStr);
        dstWstr.append(2, '\0');
        sf.pFrom = srcWstr.c_str();
        sf.pTo = dstWstr.c_str();

        FCM::AutoPtr<FCM::IFCMCalloc> pCalloc = GetCallocService(pCallback);
        int n = SHFileOperation(&sf);
        if (n != 0)
        {
            pCalloc->Free(srcFolderStr);
            pCalloc->Free(dstFolderStr);
            return FCM_GENERAL_ERROR;
        }

        pCalloc->Free(srcFolderStr);
        pCalloc->Free(dstFolderStr);
#else

        copyfile(srcFolder.c_str(), dstFolder.c_str(), NULL, COPYFILE_ALL | COPYFILE_RECURSIVE);
#endif
        return FCM_SUCCESS;
    }


#ifdef USE_HTTP_SERVER

    void Utils::LaunchBrowser(const std::string& outputFileName, int port, FCM::PIFCMCallback pCallback)
    {

#ifdef _WINDOWS

        std::wstring output = L"http://localhost:";
        std::wstring tail;
        tail.assign(outputFileName.begin(), outputFileName.end());
        FCM::StringRep16 portStr = Utils::ToString16(Utils::ToString(port), pCallback);
        output += portStr;
        output += L"/";
        output += tail;
        ShellExecute(NULL, L"open", output.c_str(), NULL, NULL, SW_SHOWNORMAL);

        FCM::AutoPtr<FCM::IFCMCalloc> pCalloc = GetCallocService(pCallback);
        pCalloc->Free(portStr);
#else

        std::string output = "http://localhost:";
        output += Utils::ToString(port);
        output += "/";
        output += outputFileName;
        std::string str = "/usr/bin/open " + output;
        popen(str.c_str(), "r");
        
#endif // _WINDOWS

    }

#endif // USE_HTTP_SERVER
}

