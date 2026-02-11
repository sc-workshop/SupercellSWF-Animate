#include "XflCache.h"
#include "Reader/File.h"
#include "core/hashing/ncrypto/xxhash.h"

namespace Animate {
	size_t XflCache::GetCache(const Path& path) {
		XFL::XFLFile file(path);

		std::vector<Path> paths = file.GetPaths();
		std::sort(paths.begin(), paths.end());

		wk::hash::XxHash hasher;

		for (auto& entry : paths) {
			auto name = entry.string();

			if (!name.starts_with("LIBRARY") && !name.starts_with("bin"))
				continue;

			if (name == "bin/SymDepend.cache")
				continue;

			auto stream = file.ReadFile(entry);
			hasher.update((uint8_t*)stream->data(), stream->length());
		}

		return hasher.digest();
	}
}