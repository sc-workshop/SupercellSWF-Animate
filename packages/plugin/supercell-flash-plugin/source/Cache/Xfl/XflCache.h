#pragma once

#include "core/generic/static.h"
#include "Reader/File.h"

namespace Animate {
	class XflCache {
		WK_STATIC_CLASS(XflCache);

	public:
		static size_t GetCache(const Path& path);
	};
}