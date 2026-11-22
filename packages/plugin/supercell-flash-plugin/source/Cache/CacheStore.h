#pragma once

#include "Cache/CacheKey.h"
#include "Cache/CacheLock.h"
#include "Cache/Generated/CacheTypes_generated.h"
#include "animate/publisher/SymbolCache.h"
#include "core/image/raw_image.h"
#include "core/io/file_stream.h"
#include "core/memory/ref.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace sc::Adobe::Cache {
    class CacheStore {
    public:
        CacheStore(const std::string& fileGuid,
                   const CacheKey& policyKey,
                   const std::string& build);
        ~CacheStore();

        CacheStore(const CacheStore&) = delete;
        CacheStore& operator=(const CacheStore&) = delete;

        bool IsEnabled() const { return m_enabled; }

        bool TryLoadImage(const CacheKey& key, const CacheKey& sourceKey, wk::RawImageRef& image);
        bool StoreImage(const CacheKey& key, const CacheKey& sourceKey, const wk::RawImage& image);

        bool TryLoadSymbol(const CacheKey& key, Animate::Publisher::CachedSymbolPayload& payload);
        bool StoreSymbol(const CacheKey& key, const Animate::Publisher::CachedSymbolPayload& payload);

        bool Commit();

        void Disable(const char* reason = "disabled");

        const std::string& FileGuid() const { return m_fileGuid; }
        const std::filesystem::path& Root() const { return m_root; }
        std::string Status() const;
        static fs::path RootDirectory();

        // Raw .fla identity and the export-root manifest let a �ached document be replayed
        // without asking Animate to load it again
        void SetDocumentFingerprint(uint64_t rawFlaHash);
        void SetRootKeys(const std::vector<CacheKey>& keys, bool complete);
        uint64_t GetHash() const;
        std::vector<CacheKey> RootKeys() const;
        bool RootsComplete() const;

    private:
        struct BlobEntry {
            Gen::BlobKind kind = Gen::BlobKind::Unknown;
            uint64_t offset = 0;
            uint32_t packedSize = 0;
            uint32_t rawSize = 0;
            uint32_t checksum = 0;
        };

        using EntryMap = std::map<CacheKey, BlobEntry>;

    private:
        bool LoadIndex();
        bool LoadPack();
        void ResetIndex();
        void DisableLocked(const char* reason);
        void Invalidate(EntryMap::iterator it, const char* reason);
        void InvalidateKey(const CacheKey& key, const BlobEntry& expected, const char* reason);
        bool ReadBlob(const BlobEntry& entry, uint64_t committedSize, std::vector<uint8_t>& blob, const char*& failure) const;
        bool AppendBlobLocked(const CacheKey& key,
                              const uint8_t* bytes,
                              size_t size,
                              size_t rawSize,
                              Gen::BlobKind kind,
                              BlobEntry& entry);

    private:
        std::string m_fileGuid;
        CacheKey m_policyKey;
        std::string m_build;

        fs::path m_root;
        fs::path m_packPath;
        fs::path m_indexPath;

        CacheLock m_lock;
        mutable std::mutex m_mutex;

        // cache.pack is one zstd frame. It is decompressed once at load; blobs are
        // addressed by their offsets in this raw image and the whole frame is rewritten
        // only when the index is committed.
        std::vector<uint8_t> m_rawPack;

        EntryMap m_entries;
        uint64_t m_rawFlaHash = 0;
        std::vector<CacheKey> m_rootKeys;
        bool m_rootsComplete = false;
        mutable std::map<std::string, uint64_t> m_invalidations;
        mutable uint64_t m_symbolLoadMicros = 0;
        mutable uint64_t m_symbolLoadCount = 0;
        mutable uint64_t m_symbolReadMicros = 0;
        mutable uint64_t m_symbolHashMicros = 0;
        mutable uint64_t m_symbolDecodeMicros = 0;
        mutable uint64_t m_imageLoadMicros = 0;
        mutable uint64_t m_imageLoadCount = 0;
        uint64_t m_generation = 1;
        uint64_t m_writeOffset = 0;
        bool m_packDirty = false;
        bool m_enabled = false;
        std::string m_status = "ok";
    };
}
