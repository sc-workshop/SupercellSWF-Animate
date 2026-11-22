#pragma once

#include "Cache/CacheKey.h"
#include "Cache/CacheStore.h"
#include "Cache/Xfl/XflDocumentInfo.h"
#include "animate/publisher/SymbolCache.h"
#include "core/image/raw_image.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace sc::Adobe {
    class SCConfig;
}

namespace sc::Adobe::Cache {
    class CacheManager {
    public:
        struct CachedDocument {
            std::string fileGuid;
            uint64_t rawHash = 0;
            std::vector<Animate::Publisher::CachedSymbolRoot> roots;
        };

        explicit CacheManager(const SCConfig& config);

        CacheManager(const CacheManager&) = delete;
        CacheManager& operator=(const CacheManager&) = delete;

        void SetDocument(const fs::path& documentPath);
        void SetRootKeys(const std::vector<CacheKey>& keys, bool complete);

        static uint64_t HashFile(const std::filesystem::path& documentPath);

        /// @brief Reads a cached document's root payloads without loading fla in Animate
        bool TryGetCachedDocument(const std::filesystem::path& documentPath, CachedDocument& result);

        /// @brief Temporarily routes cache reads to one cached document during replay
        void BeginReplay(const std::string& fileGuid);
        void EndReplay();

        bool GetImageKey(const std::u16string& name, CacheKey& cacheKey, CacheKey& sourceKey) const;

        bool TryLoadImage(const CacheKey& key, const CacheKey& sourceKey, wk::RawImageRef& image) const;
        bool StoreImage(const CacheKey& key, const CacheKey& sourceKey, const wk::RawImage& image) const;

        bool GetSymbolKey(const Animate::Publisher::SymbolContext& symbol, CacheKey& cacheKey) const;

        bool TryLoadSymbol(const CacheKey& key, Animate::Publisher::CachedSymbolPayload& payload) const;
        bool StoreSymbol(const CacheKey& key, const Animate::Publisher::CachedSymbolPayload& payload);

        const CacheKey& PolicyKey() const { return m_policyKey; }

        void Commit();

        bool IsEnabled() const;
        std::string ActiveFileGuid() const;
        std::string Summary() const;
        std::string UserSummary() const;

    private:
        CacheKey m_policyKey;
        std::string m_build;
        std::string m_status = "not initialized";

        mutable std::mutex m_mutex;
        std::unordered_map<std::string, std::shared_ptr<CacheStore>> m_stores;
        std::shared_ptr<CacheStore> m_activeStore;
        std::vector<std::shared_ptr<CacheStore>> m_replayStack;
        XflDocumentInfo m_document;
        bool m_documentValid = false;
        
        mutable uint64_t m_imageHits = 0;
        mutable uint64_t m_imageMisses = 0;
        mutable uint64_t m_imageStores = 0;
        mutable uint64_t m_symbolHits = 0;
        mutable uint64_t m_symbolMisses = 0;
        mutable uint64_t m_symbolStores = 0;

        mutable uint64_t m_xflReadMicros = 0;
        mutable uint64_t m_imageKeyMicros = 0;
        mutable uint64_t m_imageKeyCount = 0;
        mutable uint64_t m_symbolKeyMicros = 0;
        mutable uint64_t m_symbolKeyCount = 0;
    };
}
