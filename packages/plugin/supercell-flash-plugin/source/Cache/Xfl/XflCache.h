#pragma once

#include "Reader/File.h"
#include "core/generic/static.h"

namespace Animate {
    class XflCache {
        WK_STATIC_CLASS(XflCache);

    public:
        static size_t GetCache(const Path& path);
    };
}