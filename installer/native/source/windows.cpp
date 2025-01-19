#include "platform.h"

std::filesystem::path get_user_cep()
{
    std::string appdata = getenv("APPDATA");
    appdata += "\\Adobe\\CEP\\";
    std::replace(appdata.begin(), appdata.end(), '\\', '/');

    return appdata;
}