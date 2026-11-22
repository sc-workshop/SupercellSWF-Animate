#pragma once

#include "Cache/CacheKey.h"

#include <cstddef>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace sc::Adobe::Cache {
    struct XflSymbolInfo {
        std::string itemId;
        std::string lastModified;

        // Direct dependencies from Animate's SymDepend cache. No symbol XML is opened.
        std::vector<std::string> dependencies;
    };

    struct XflDocumentInfo {
        std::string fileGuid;

        // Cheap metadata fingerprint of XFL entries. Used only as a fallback for inline
        // resources with no authored bitmap entry; never for symbol identity.
        uint64_t hash = 0;

        std::unordered_map<std::u16string, CacheKey> bitmapKeys;
        std::unordered_map<std::string, CacheKey> bitmapKeysUtf8;

        // Authoring metadata from DOMDocument.xml and the persisted dependency sidebar.
        std::unordered_map<std::string, XflSymbolInfo> symbols;

        // Linkage class names from symbol XML headers, cached in symbol-meta.index.
        std::unordered_map<std::string, std::string> linkages;

        std::unordered_map<std::string, CacheKey> symbolHashes;
        std::unordered_map<std::string, CacheKey> symbolHashesByLinkage;
    };

    class XflDocumentReader {
    public:
        static bool TryRead(const std::filesystem::path& documentPath, XflDocumentInfo& result);
        static std::string LastError();
    };
}
