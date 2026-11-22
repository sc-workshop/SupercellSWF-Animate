#include "CacheManager.h"

#include "Module/Config.h"
#include "Module/Module.h"
#include "Module/Version.h"
#include "core/asset_manager/asset_manager.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <format>
#include <vector>

namespace sc::Adobe::Cache {
    namespace {
#if defined(NDEBUG)
        constexpr const char* CacheBuild = __SC_ANIMATE_VERSION__;
#else
        constexpr const char* CacheBuild = __DATE__ " " __TIME__;
#endif

        void AddBool(CacheKeyBuilder& builder, bool value) {
            const uint8_t byte = value ? 1 : 0;
            builder.updateScalar(byte);
        }

        std::u16string NormalizeName(std::u16string value) {
            for (char16_t& c : value) {
                if (c == u'\\')
                    c = u'/';
            }

            while (value.size() >= 2 && value[0] == u'.' && value[1] == u'/') {
                value.erase(0, 2);
            }

            return value;
        }

        std::string NormalizeName(std::string value) {
            std::replace(value.begin(), value.end(), '\\', '/');
            while (value.size() >= 2 && value[0] == '.' && value[1] == '/') {
                value.erase(0, 2);
            }
            return value;
        }

        CacheKey CacheKeyFromSymbolKey(const Animate::Publisher::SymbolCacheKey& key) {
            CacheKey result {};
            std::copy(key.begin(), key.end(), result.begin());
            return result;
        }

        Animate::Publisher::SymbolCacheKey SymbolKeyFromCacheKey(const CacheKey& key) {
            Animate::Publisher::SymbolCacheKey result {};
            std::copy(key.begin(), key.end(), result.begin());
            return result;
        }

        uint64_t HashFileBytes(const std::filesystem::path& path) {
            auto& manager = wk::AssetManager::Instance();
            if (!manager.exists(path))
                return 0;

            auto file = manager.load_file(path);
            wk::hash::XxHash hasher;
            std::array<uint8_t, 32 * 1024> buffer {};

            file->seek(0);
            size_t remaining = file->length();
            while (remaining != 0) {
                const size_t count = std::min(buffer.size(), remaining);
                if (file->read(buffer.data(), count) != count)
                    return 0;

                hasher.update(buffer.data(), count);
                remaining -= count;
            }

            return hasher.digest();
        }

        CacheKey BuildSymbolSourceKey(const XflDocumentInfo& document,
                                      const Animate::Publisher::SymbolContext& symbol) {
            CacheKeyBuilder builder;

            // Identity comes from this symbol's XFL XML and its dependency subtree
            const std::string name = wk::StringConverter::ToUTF8(symbol.name);
            const CacheKey* symbolHash = nullptr;
            if (auto found = document.symbolHashes.find(NormalizeName(name)); found != document.symbolHashes.end()) {
                symbolHash = &found->second;
            } else if (!symbol.linkage_name.empty()) {
                if (auto byLinkage = document.symbolHashesByLinkage.find(symbol.linkage_name);
                    byLinkage != document.symbolHashesByLinkage.end()) {
                    symbolHash = &byLinkage->second;
                }
            }

            if (symbolHash) {
                builder.update(*symbolHash);
            } else {
                // Inline resources derive their keys from the owning symbol. Authored symbols
                // missing from the dependency map still stay stable by name/linkage alone.
                builder.update(std::string("inline"));
                builder.update(name);
            }

            builder.update(symbol.linkage_name);
            builder.updateScalar(symbol.type);
            builder.updateScalar(symbol.looping.GetMode());
            builder.updateScalar(symbol.looping.GetStartFrame());
            builder.updateScalar(symbol.looping.GetEndFrame());
            builder.updateScalar(symbol.looping.GetFrameCount());
            builder.updateScalar(symbol.slicing.IsEnabled());
            if (symbol.slicing.IsEnabled()) {
                const auto& guides = symbol.slicing.Guides();
                builder.updateScalar(guides.topLeft.x);
                builder.updateScalar(guides.topLeft.y);
                builder.updateScalar(guides.bottomRight.x);
                builder.updateScalar(guides.bottomRight.y);
            }
            return builder.finish();
        }

        CacheKey BuildPolicyKey(const SCConfig& config) {
            CacheKeyBuilder builder;

            builder.update(std::string(CacheBuild));

            // Only settings that change captured symbol/image bytes belong here.
            // Publish mode, output paths, atlas/texture settings, compression, precision,
            // frame encoding, fields text and custom properties are output-only: text and
            // custom properties are always captured, then stripped while writing the .sc.
            // A document therefore shares one cache between single-document and
            // multi-document publishes regardless of those settings.
            AddBool(builder, config.aggressiveRasterization);
            AddBool(builder, config.backwardCompatibility);

            return builder.finish();
        }
    }

    CacheManager::CacheManager(const SCConfig& config) :
        m_policyKey(BuildPolicyKey(config)),
        m_build(CacheBuild) {
    }

    void CacheManager::SetDocument(const std::filesystem::path& documentPath) {
        SCPlugin& diagContext = SCPlugin::Instance();
        diagContext.logger->info("[Perf] CacheManager::SetDocument enter path={}", documentPath.string());

        const auto xflStart = std::chrono::steady_clock::now();
        XflDocumentInfo document;
        const bool xflRead = XflDocumentReader::TryRead(documentPath, document);
        const auto xflMicros = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - xflStart).count());

        diagContext.logger->info("[Perf] XflDocumentReader::TryRead={}ms ok={}", xflMicros / 1000, xflRead ? 1 : 0);
        if (!xflRead) {
            const std::string reason = XflDocumentReader::LastError();
            SCPlugin& context = SCPlugin::Instance();
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_xflReadMicros += xflMicros;
                m_activeStore.reset();
                m_document = {};
                m_documentValid = false;
                m_status = std::format("disabled: {}", reason.empty() ? "XFL unreadable" : reason);
            }
            context.logger->info("[Cache] disabled for '{}': {}", documentPath.string(), m_status);
            return;
        }

        const std::string fileGuid = document.fileGuid;
        // The XFL marker file is only a few bytes. The reader's document hash covers the real
        // DOMDocument.xml and library contents, so it is the only fingerprint that can invalidate
        // the cache when the animation changes.
        const uint64_t rawHash = document.hash;
        std::shared_ptr<CacheStore> store;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_xflReadMicros += xflMicros;
            auto it = m_stores.find(fileGuid);
            if (it == m_stores.end()) {
                store = std::make_shared<CacheStore>(fileGuid, m_policyKey, m_build);
                m_stores.emplace(fileGuid, store);
            } else {
                store = it->second;
            }

            m_document = std::move(document);
            m_documentValid = true;
            m_activeStore = store;
            store->SetDocumentFingerprint(rawHash);
            m_status = std::format("{}: {}", store->IsEnabled() ? "enabled" : "disabled", store->Status());

            if (!store->IsEnabled()) {
                m_activeStore.reset();
            }
        }

        SCPlugin& context = SCPlugin::Instance();
        context.logger->info("[Cache] fileGUID={} key={} root={} status={}",
                             fileGuid,
                             ToHex(m_policyKey),
                             store->Root().string(),
                             m_status);
    }

    uint64_t CacheManager::HashFile(const std::filesystem::path& documentPath) {
        return HashFileBytes(documentPath);
    }

    void CacheManager::SetRootKeys(const std::vector<CacheKey>& keys, bool complete) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_activeStore && m_activeStore->IsEnabled())
            m_activeStore->SetRootKeys(keys, complete);
    }

    bool CacheManager::TryGetCachedDocument(const std::filesystem::path& documentPath, CachedDocument& result) {
        XflDocumentInfo info;
        if (!XflDocumentReader::TryRead(documentPath, info) || info.fileGuid.empty() || info.hash == 0)
            return false;

        const uint64_t rawHash = info.hash;

        std::shared_ptr<CacheStore> store;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_stores.find(info.fileGuid);
            if (it == m_stores.end()) {
                store = std::make_shared<CacheStore>(info.fileGuid, m_policyKey, m_build);
                m_stores.emplace(info.fileGuid, store);
            } else {
                store = it->second;
            }
        }

        if (!store->IsEnabled() || store->GetHash() != rawHash || !store->RootsComplete())
            return false;

        std::vector<Animate::Publisher::CachedSymbolRoot> roots;
        for (const CacheKey& key : store->RootKeys()) {
            Animate::Publisher::CachedSymbolPayload payload;
            if (!store->TryLoadSymbol(key, payload))
                return false;

            Animate::Publisher::CachedSymbolRoot root;
            root.key = SymbolKeyFromCacheKey(key);
            root.payload = std::move(payload);
            roots.push_back(std::move(root));
        }

        result.fileGuid = info.fileGuid;
        result.rawHash = rawHash;
        result.roots = std::move(roots);
        return true;
    }

    void CacheManager::BeginReplay(const std::string& fileGuid) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_stores.find(fileGuid);
        if (it == m_stores.end())
            return;
        m_replayStack.push_back(m_activeStore);
        m_activeStore = it->second;
    }

    void CacheManager::EndReplay() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_replayStack.empty()) {
            m_activeStore = m_replayStack.back();
            m_replayStack.pop_back();
        }
    }

    bool CacheManager::GetImageKey(const std::u16string& name, CacheKey& cacheKey, CacheKey& sourceKey) const {
        const auto start = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_documentValid || !m_activeStore || !m_activeStore->IsEnabled())
            return false;

        const std::u16string lookupName = NormalizeName(name);
        auto it = m_document.bitmapKeys.find(lookupName);
        if (it != m_document.bitmapKeys.end()) {
            sourceKey = it->second;
        } else {
            CacheKeyBuilder builder;
            builder.updateScalar(m_document.hash);
            builder.update(lookupName);
            sourceKey = builder.finish();
        }

        cacheKey = DeriveCacheKey(m_policyKey, "image", sourceKey);
        m_imageKeyMicros += static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count());
        ++m_imageKeyCount;
        return true;
    }

    bool CacheManager::TryLoadImage(const CacheKey& key, const CacheKey& sourceKey, wk::RawImageRef& image) const {
        std::shared_ptr<CacheStore> store;
        std::vector<std::shared_ptr<CacheStore>> otherStores;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            store = m_activeStore;
            otherStores.reserve(m_stores.size());
            for (const auto& entry : m_stores) {
                if (entry.second != store)
                    otherStores.push_back(entry.second);
            }
        }

        bool loaded = store && store->IsEnabled() && store->TryLoadImage(key, sourceKey, image);
        if (!loaded) {
            for (const auto& other : otherStores) {
                if (other && other->IsEnabled() && other->TryLoadImage(key, sourceKey, image)) {
                    loaded = true;
                    break;
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (loaded) {
                ++m_imageHits;
            } else {
                ++m_imageMisses;
            }
        }

        return loaded;
    }

    bool CacheManager::StoreImage(const CacheKey& key, const CacheKey& sourceKey, const wk::RawImage& image) const {
        std::shared_ptr<CacheStore> store;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            store = m_activeStore;
        }

        const bool stored = store && store->IsEnabled() && store->StoreImage(key, sourceKey, image);

        if (stored) {
            std::lock_guard<std::mutex> lock(m_mutex);
            ++m_imageStores;
        }

        return stored;
    }

    bool CacheManager::GetSymbolKey(const Animate::Publisher::SymbolContext& symbol, CacheKey& cacheKey) const {
        const auto start = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_documentValid || !m_activeStore || !m_activeStore->IsEnabled())
            return false;

        const CacheKey sourceKey = BuildSymbolSourceKey(m_document, symbol);
        cacheKey = DeriveCacheKey(m_policyKey, "symbol", sourceKey);
        m_symbolKeyMicros += static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count());
        ++m_symbolKeyCount;
        return true;
    }

    bool CacheManager::TryLoadSymbol(const CacheKey& key, Animate::Publisher::CachedSymbolPayload& payload) const {
        std::shared_ptr<CacheStore> store;
        std::vector<std::shared_ptr<CacheStore>> otherStores;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            store = m_activeStore;
            // A cached symbol can reference a resource/symbol owned by another document.
            // Those blobs live in that document's store; keep the active store first but
            // fall back to the others so cross-document replays resolve.
            otherStores.reserve(m_stores.size());
            for (const auto& entry : m_stores) {
                if (entry.second != store)
                    otherStores.push_back(entry.second);
            }
        }

        bool loaded = store && store->IsEnabled() && store->TryLoadSymbol(key, payload);
        if (!loaded) {
            for (const auto& other : otherStores) {
                if (other && other->IsEnabled() && other->TryLoadSymbol(key, payload)) {
                    loaded = true;
                    break;
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (loaded) {
                ++m_symbolHits;
            } else {
                ++m_symbolMisses;
            }
        }

        return loaded;
    }

    bool CacheManager::StoreSymbol(const CacheKey& key, const Animate::Publisher::CachedSymbolPayload& payload) {
        std::shared_ptr<CacheStore> store;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            store = m_activeStore;
        }

        const bool stored = store && store->IsEnabled() && store->StoreSymbol(key, payload);

        if (stored) {
            std::lock_guard<std::mutex> lock(m_mutex);
            ++m_symbolStores;
        }

        return stored;
    }

    void CacheManager::Commit() {
        SCPlugin& context = SCPlugin::Instance();
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (auto& pair : m_stores) {
                const bool committed = pair.second->Commit();
                context.logger->info("[Cache] commit fileGUID={} ok={} status={}",
                                     pair.first,
                                     committed,
                                     pair.second->Status());
            }
        }

        context.logger->info("[Cache] {}", Summary());
    }

    bool CacheManager::IsEnabled() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_documentValid && m_activeStore && m_activeStore->IsEnabled();
    }

    std::string CacheManager::ActiveFileGuid() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_documentValid ? m_document.fileGuid : std::string();
    }

    std::string CacheManager::Summary() const {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::string status = m_status;
        if (m_activeStore && m_activeStore->IsEnabled())
            status = std::format("enabled: {}", m_activeStore->Status());

        std::string result = std::
            format("fileGUID={}; {}; images: {} hit, {} miss, {} stored; symbols: {} hit, {} miss, {} stored; xfl={}ms",
                   m_documentValid ? m_document.fileGuid : "<none>",
                   status,
                   m_imageHits,
                   m_imageMisses,
                   m_imageStores,
                   m_symbolHits,
                   m_symbolMisses,
                   m_symbolStores,
                   m_xflReadMicros / 1000);

        if (m_imageKeyCount != 0)
            result += std::format("; image-key={}ms/{}", m_imageKeyMicros / 1000, m_imageKeyCount);

        if (m_symbolKeyCount != 0)
            result += std::format("; symbol-key={}ms/{}", m_symbolKeyMicros / 1000, m_symbolKeyCount);

        if (m_activeStore)
            result += std::format("; root={}", m_activeStore->Root().string());

        return result;
    }

    // Short, non-technical version shown in the progress window. Summary() keeps the
    // fileGUID/policy-folder details for the text log only.
    std::string CacheManager::UserSummary() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_activeStore || !m_activeStore->IsEnabled())
            return "Cache: off";

        const uint64_t symbolHits = m_symbolHits;
        const uint64_t symbolMisses = m_symbolMisses;
        const uint64_t imageHits = m_imageHits;
        const uint64_t imageMisses = m_imageMisses;

        std::string result =
            std::format("Cache: {} symbols reused, {} rebuilt, {} saved", symbolHits, symbolMisses, m_symbolStores);
        if (imageHits != 0 || imageMisses != 0 || m_imageStores != 0) {
            result += std::format("; {} images reused, {} loaded, {} saved", imageHits, imageMisses, m_imageStores);
        }

        return result;
    }
}
