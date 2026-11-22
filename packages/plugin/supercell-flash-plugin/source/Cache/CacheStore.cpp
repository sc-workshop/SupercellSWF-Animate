#include "CacheStore.h"

#include "Cache/Generated/CacheImage_generated.h"
#include "Cache/Generated/CacheIndex_generated.h"
#include "Cache/Generated/CacheSymbol_generated.h"
#include "core/asset_manager/asset_manager.h"
#include "core/hashing/ncrypto/fnv.h"
#include "core/image/image.h"
#include "core/io/shared_memory_stream.h"
#include "core/memory/ref.h"
#include "core/string/string_converter.h"
#include "compression/backend/zstd/zstd_compressor.h"
#include "compression/backend/zstd/zstd_decompressor.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <format>
#include <limits>
#include <system_error>

#if defined(_WIN32) || defined(_WINDOWS)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
    #undef LoadImage
#endif

namespace sc::Adobe::Cache {
    namespace {
        constexpr int CacheCompressionLevel = 1;
        constexpr int CacheCompressionWorkers = 4;

        bool Equals(const flatbuffers::String* value, const std::string& expected) {
            return value && value->str() == expected;
        }

        bool Equals(const flatbuffers::Vector<uint8_t>* value, const CacheKey& expected) {
            return value && value->size() == expected.size() &&
                   std::equal(value->begin(), value->end(), expected.begin());
        }

        std::vector<uint8_t> ReadFileBytes(const fs::path& path) {
            auto& manager = wk::AssetManager::Instance();
            if (!manager.exists(path))
                return {};

            auto file = manager.load_file(path);
            std::vector<uint8_t> result(file->length());
            file->seek(0);
            if (result.size() != 0 && file->read(result.data(), result.size()) != result.size()) {
                return {};
            }

            return result;
        }

        bool ReplaceFileWith(const fs::path& source, const fs::path& target) {
#if defined(_WIN32) || defined(_WINDOWS)
            return MoveFileExW(source.wstring().c_str(), target.wstring().c_str(), MOVEFILE_REPLACE_EXISTING) != FALSE;
#else
            std::error_code error;
            std::filesystem::rename(source, target, error);
            return !error;
#endif
        }

        Gen::CachedPixelDepth ToCachedDepth(wk::Image::PixelDepth depth) {
            return static_cast<Gen::CachedPixelDepth>(depth);
        }

        wk::Image::PixelDepth FromCachedDepth(Gen::CachedPixelDepth depth) {
            const auto value = static_cast<uint16_t>(depth);
            if (value > static_cast<uint16_t>(wk::Image::PixelDepth::LUMINANCE8)) {
                return wk::Image::PixelDepth::RGBA8;
            }
            return static_cast<wk::Image::PixelDepth>(value);
        }

        Gen::CachedColorSpace ToCachedColorSpace(wk::Image::ColorSpace space) {
            return space == wk::Image::ColorSpace::sRGB ? Gen::CachedColorSpace::SRGB : Gen::CachedColorSpace::Linear;
        }

        wk::Image::ColorSpace FromCachedColorSpace(Gen::CachedColorSpace space) {
            return space == Gen::CachedColorSpace::SRGB ? wk::Image::ColorSpace::sRGB : wk::Image::ColorSpace::Linear;
        }

        Gen::SymbolKind ToCachedSymbolKind(Animate::Publisher::WriterType type) {
            using Animate::Publisher::WriterType;
            switch (type) {
                case WriterType::MovieClip:
                    return Gen::SymbolKind::MovieClip;
                case WriterType::TextField:
                    return Gen::SymbolKind::TextField;
                case WriterType::Graphic:
                default:
                    return Gen::SymbolKind::Graphic;
            }
        }

        bool FromCachedSymbolKind(Gen::SymbolKind kind, Animate::Publisher::WriterType& type) {
            using Animate::Publisher::WriterType;
            switch (kind) {
                case Gen::SymbolKind::Graphic:
                    type = WriterType::Graphic;
                    return true;
                case Gen::SymbolKind::MovieClip:
                    type = WriterType::MovieClip;
                    return true;
                case Gen::SymbolKind::TextField:
                    type = WriterType::TextField;
                    return true;
                default:
                    return false;
            }
        }

        Gen::RequestKind ToCachedRequestKind(Animate::Publisher::CachedSymbolRequest::Kind kind) {
            using Animate::Publisher::CachedSymbolRequest;
            switch (kind) {
                case CachedSymbolRequest::Kind::Symbol:
                    return Gen::RequestKind::Symbol;
                case CachedSymbolRequest::Kind::Modifier:
                    return Gen::RequestKind::Modifier;
                case CachedSymbolRequest::Kind::Resource:
                default:
                    return Gen::RequestKind::Resource;
            }
        }

        bool FromCachedRequestKind(Gen::RequestKind kind, Animate::Publisher::CachedSymbolRequest::Kind& result) {
            using Animate::Publisher::CachedSymbolRequest;
            switch (kind) {
                case Gen::RequestKind::Symbol:
                    result = CachedSymbolRequest::Kind::Symbol;
                    return true;
                case Gen::RequestKind::Resource:
                    result = CachedSymbolRequest::Kind::Resource;
                    return true;
                case Gen::RequestKind::Modifier:
                    result = CachedSymbolRequest::Kind::Modifier;
                    return true;
                default:
                    return false;
            }
        }

        Gen::ModifierKind ToCachedModifierKind(Animate::Publisher::MaskedLayerState type) {
            using Animate::Publisher::MaskedLayerState;
            switch (type) {
                case MaskedLayerState::MASKED_LAYERS:
                    return Gen::ModifierKind::Masked;
                case MaskedLayerState::MASKED_LAYERS_END:
                    return Gen::ModifierKind::Unmasked;
                case MaskedLayerState::MASK_LAYER:
                default:
                    return Gen::ModifierKind::Mask;
            }
        }

        bool FromCachedModifierKind(Gen::ModifierKind kind, Animate::Publisher::MaskedLayerState& type) {
            using Animate::Publisher::MaskedLayerState;
            switch (kind) {
                case Gen::ModifierKind::Mask:
                    type = MaskedLayerState::MASK_LAYER;
                    return true;
                case Gen::ModifierKind::Masked:
                    type = MaskedLayerState::MASKED_LAYERS;
                    return true;
                case Gen::ModifierKind::Unmasked:
                    type = MaskedLayerState::MASKED_LAYERS_END;
                    return true;
                default:
                    return false;
            }
        }
    }

    CacheStore::CacheStore(const std::string& fileGuid,
                           const CacheKey& policyKey,
                           const std::string& build) :
        m_fileGuid(fileGuid),
        m_policyKey(policyKey),
        m_build(build) {
        std::error_code error;
        // Keep documents grouped, but isolate capture-affecting publish policies into their
        // own cache folders so switching settings never overwrites another policy's blobs.
        m_root = RootDirectory() / m_fileGuid / ToHex(m_policyKey);
        fs::create_directories(m_root, error);
        if (error) {
            Disable("cannot create cache directory");
            return;
        }

        m_packPath = m_root / "cache.pack";
        m_indexPath = m_root / "cache.index";

        if (!m_lock.TryLock(m_root / "cache.lock")) {
            Disable("cache directory is locked by another publish");
            return;
        }

        if (!LoadIndex()) {
            Disable("cannot load cache index");
            return;
        }

        if (!LoadPack()) {
            Disable("cannot load cache.pack");
            return;
        }

        m_enabled = true;
    }

    CacheStore::~CacheStore() {
        m_lock.Unlock();
    }

    void CacheStore::Disable(const char* reason) {
        std::lock_guard<std::mutex> lock(m_mutex);
        DisableLocked(reason);
    }

    void CacheStore::DisableLocked(const char* reason) {
        m_enabled = false;
        m_status = reason;
        m_entries.clear();
        m_lock.Unlock();
    }

    fs::path CacheStore::RootDirectory() {
#if defined(_WIN32) || defined(_WINDOWS)
        if (const char* localAppData = std::getenv("LOCALAPPDATA")) {
            if (*localAppData) {
                return fs::path(localAppData) / "SupercellSWF" / "Cache";
            }
        }
#else
        if (const char* home = std::getenv("HOME")) {
            if (*home) {
                return fs::path(home) / "Library" / "Application Support" / "SupercellSWF" / "Cache";
            }
        }
#endif

        return fs::temp_directory_path() / "SupercellSWF" / "Cache";
    }

    bool CacheStore::LoadIndex() {
        std::lock_guard<std::mutex> lock(m_mutex);

        const std::vector<uint8_t> bytes = ReadFileBytes(m_indexPath);
        if (bytes.size() < 8 || !Gen::CacheIndexBufferHasIdentifier(bytes.data())) {
            m_status = "new cache index";
            ResetIndex();
            return true;
        }

        flatbuffers::Verifier verifier(bytes.data(), bytes.size());
        if (!Gen::VerifyCacheIndexBuffer(verifier)) {
            m_status = "corrupt cache.index; index reset";
            ResetIndex();
            return true;
        }

        const Gen::CacheIndex* index = Gen::GetCacheIndex(bytes.data());
        if (!index || !Equals(index->file_guid(), m_fileGuid) || !Equals(index->global_key(), m_policyKey) ||
            !Equals(index->build(), m_build)) {
            m_status = "cache index does not match document/build/config; index reset";
            ResetIndex();
            return true;
        }

        m_generation = index->generation() + 1;
        m_writeOffset = index->committed_size();
        m_entries.clear();
        m_rawFlaHash = index->raw_fla_hash();
        m_rootsComplete = index->roots_complete();
        m_rootKeys.clear();

        if (const auto* rootKeys = index->root_keys()) {
            for (const Gen::RootKey* stored : *rootKeys) {
                if (!stored || !stored->key() || stored->key()->size() != m_policyKey.size())
                    continue;
                CacheKey key {};
                std::copy(stored->key()->begin(), stored->key()->end(), key.begin());
                m_rootKeys.push_back(key);
            }
        }

        if (const auto* entries = index->entries()) {
            for (const Gen::BlobEntry* entry : *entries) {
                if (!entry ||
                    (entry->kind() != Gen::BlobKind::Image && entry->kind() != Gen::BlobKind::Shape &&
                     entry->kind() != Gen::BlobKind::TextField && entry->kind() != Gen::BlobKind::MovieClip)) {
                    continue;
                }

                const auto* key = entry->key();
                if (!key || key->size() != m_policyKey.size())
                    continue;
                if (entry->packed_size() == 0 || entry->offset() + entry->packed_size() > m_writeOffset)
                    continue;

                CacheKey cacheKey {};
                std::copy(key->begin(), key->end(), cacheKey.begin());

                BlobEntry value;
                value.kind = entry->kind();
                value.offset = entry->offset();
                value.packedSize = entry->packed_size();
                value.rawSize = entry->raw_size();
                value.checksum = entry->checksum();
                m_entries.emplace(cacheKey, value);
            }
        }

        m_status = std::format("loaded {} entries", m_entries.size());
        return true;
    }

    void CacheStore::ResetIndex() {
        m_entries.clear();
        m_rawFlaHash = 0;
        m_rootKeys.clear();
        m_rootsComplete = false;
        m_invalidations.clear();
        m_symbolLoadMicros = 0;
        m_symbolLoadCount = 0;
        m_symbolReadMicros = 0;
        m_symbolHashMicros = 0;
        m_symbolDecodeMicros = 0;
        m_imageLoadMicros = 0;
        m_imageLoadCount = 0;
        m_generation = 1;
        m_writeOffset = 0;
        m_packDirty = false;
    }

    bool CacheStore::LoadPack() {
        auto reset = [&](const char* reason) {
            m_status = reason;
            ResetIndex();
            m_rawPack.clear();
        };

        if (m_writeOffset == 0) {
            m_rawPack.clear();
            return true;
        }
        if (m_writeOffset > std::numeric_limits<size_t>::max()) {
            reset("index reset: cache.pack is too large for this process");
            return true;
        }

        std::error_code error;
        if (!fs::exists(m_packPath, error) || error) {
            reset("index reset: cache.pack is missing");
            return true;
        }

        // One streaming decompress at startup. Every blob is served from the raw image,
        // so warm publishes never decompress the pack once per symbol/image.
        try {
            wk::InputFileStream packed(m_packPath);
            if (!packed.is_open()) {
                reset("index reset: cannot open cache.pack");
                return true;
            }
            if (!sc::ZstdDecompressor::validate(packed)) {
                reset("index reset: cache.pack is not a zstd frame");
                return true;
            }

            m_rawPack.resize(static_cast<size_t>(m_writeOffset));
            wk::SharedMemoryStream raw(m_rawPack.data(), m_rawPack.size());
            sc::ZstdDecompressor decompressor;
            decompressor.decompress(packed, raw);
            if (raw.position() != m_rawPack.size()) {
                reset("index reset: cache.pack has unexpected size");
                return true;
            }
        } catch (...) {
            reset("index reset: cache.pack decompression failed");
        }
        return true;
    }

    bool CacheStore::ReadBlob(const BlobEntry& entry,
                              uint64_t committedSize,
                              std::vector<uint8_t>& blob,
                              const char*& failure) const {
        if (entry.packedSize == 0 || entry.offset + entry.packedSize > committedSize ||
            entry.offset > m_rawPack.size() || entry.packedSize > m_rawPack.size() - entry.offset) {
            failure = "blob-size";
            return false;
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        const auto begin = m_rawPack.begin() + static_cast<std::ptrdiff_t>(entry.offset);
        blob.assign(begin, begin + static_cast<std::ptrdiff_t>(entry.packedSize));
        return true;
    }

    bool CacheStore::AppendBlobLocked(const CacheKey& key,
                                      const uint8_t* bytes,
                                      size_t size,
                                      size_t rawSize,
                                      Gen::BlobKind kind,
                                      BlobEntry& entry) {
        if (size == 0 || size > std::numeric_limits<uint32_t>::max() ||
            rawSize > std::numeric_limits<uint32_t>::max() || m_rawPack.size() != m_writeOffset) {
            DisableLocked("cache.pack size desync");
            return false;
        }

        m_rawPack.insert(m_rawPack.end(), bytes, bytes + size);

        entry.kind = kind;
        entry.offset = m_writeOffset;
        entry.packedSize = static_cast<uint32_t>(size);
        entry.rawSize = static_cast<uint32_t>(rawSize);
        entry.checksum = wk::hash::fnv1a32(bytes, size);

        m_entries.emplace(key, entry);
        m_writeOffset += size;
        m_packDirty = true;
        return true;
    }

    void CacheStore::SetDocumentFingerprint(uint64_t rawFlaHash) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_rawFlaHash = rawFlaHash;
    }

    void CacheStore::SetRootKeys(const std::vector<CacheKey>& keys, bool complete) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_rootKeys = keys;
        m_rootsComplete = complete;
    }

    uint64_t CacheStore::GetHash() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_rawFlaHash;
    }

    std::vector<CacheKey> CacheStore::RootKeys() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_rootKeys;
    }

    bool CacheStore::RootsComplete() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_rootsComplete;
    }

    std::string CacheStore::Status() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string result = m_status;
        uint64_t total = 0;
        for (const auto& pair : m_invalidations)
            total += pair.second;
        if (m_symbolLoadCount != 0)
            result += std::format("; symbol-load={}ms/{} (read={} hash={} decode={})",
                                  m_symbolLoadMicros / 1000,
                                  m_symbolLoadCount,
                                  m_symbolReadMicros / 1000,
                                  m_symbolHashMicros / 1000,
                                  m_symbolDecodeMicros / 1000);

        if (m_imageLoadCount != 0)
            result += std::format("; image-load={}ms/{}", m_imageLoadMicros / 1000, m_imageLoadCount);

        if (total == 0)
            return result;

        result += std::format("; invalidated {} (", total);
        bool first = true;
        for (const auto& pair : m_invalidations) {
            if (!first)
                result += ", ";
            first = false;
            result += std::format("{}={}", pair.first, pair.second);
        }
        result += ")";
        return result;
    }

    void CacheStore::Invalidate(EntryMap::iterator it, const char* reason) {
        ++m_invalidations[reason];
        m_entries.erase(it);
    }

    void CacheStore::InvalidateKey(const CacheKey& key, const BlobEntry& expected, const char* reason) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_entries.find(key);
        if (it == m_entries.end() || it->second.offset != expected.offset ||
            it->second.packedSize != expected.packedSize || it->second.checksum != expected.checksum)
            return;
        Invalidate(it, reason);
    }

    bool CacheStore::TryLoadImage(const CacheKey& key, const CacheKey& sourceKey, wk::RawImageRef& image) {
        const auto start = std::chrono::steady_clock::now();

        BlobEntry entry;
        uint64_t committedSize = 0;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_enabled)
                return false;

            auto it = m_entries.find(key);
            if (it == m_entries.end())
                return false;

            entry = it->second;
            committedSize = m_writeOffset;
        }

        bool loaded = false;
        const char* failure = nullptr;

        // Blob validation is pure. Keep it outside m_mutex so image loads can run in parallel;
        // the checksum identifies the exact bytes this store wrote.
        try {
            std::vector<uint8_t> blob;
            const char* readFailure = nullptr;
            if (!ReadBlob(entry, committedSize, blob, readFailure)) {
                failure = readFailure ? readFailure : "image-read";
            } else if (blob.size() < 8 || !Gen::CachedImageBlobBufferHasIdentifier(blob.data())) {
                failure = "image-identifier";
            } else if (wk::hash::fnv1a32(blob.data(), blob.size()) != entry.checksum) {
                failure = "image-checksum";
            } else {
                const Gen::CachedImageBlob* wrapper = Gen::GetCachedImageBlob(blob.data());
                const Gen::CachedImage* cached = wrapper ? wrapper->image() : nullptr;
                const auto* storedKey = wrapper ? wrapper->key() : nullptr;
                const auto* storedSourceKey = cached ? cached->source_key() : nullptr;
                if (!cached || !storedKey || storedKey->size() != key.size() ||
                    !std::equal(storedKey->begin(), storedKey->end(), key.begin()) || !storedSourceKey ||
                    storedSourceKey->size() != sourceKey.size() ||
                    !std::equal(storedSourceKey->begin(), storedSourceKey->end(), sourceKey.begin())) {
                    failure = "image-key";
                } else {
                    const uint32_t width = cached->width();
                    const uint32_t height = cached->height();
                    if (width == 0 || height == 0 || width > std::numeric_limits<wk::Image::SizeT>::max() ||
                        height > std::numeric_limits<wk::Image::SizeT>::max()) {
                        failure = "image-size";
                    } else {
                        const wk::Image::PixelDepth depth = FromCachedDepth(cached->depth());
                        const wk::Image::ColorSpace space = FromCachedColorSpace(cached->colorspace());
                        const auto* data = cached->data();
                        const size_t expected = wk::Image::calculate_image_length(static_cast<wk::Image::SizeT>(width),
                                                                                  static_cast<wk::Image::SizeT>(height),
                                                                                  depth);

                        if (!data || data->size() != expected || expected != entry.rawSize) {
                            failure = "image-data";
                        } else {
                            auto result = wk::CreateRef<wk::RawImage>(static_cast<wk::Image::SizeT>(width),
                                                                      static_cast<wk::Image::SizeT>(height),
                                                                      depth,
                                                                      space);
                            std::copy(data->begin(), data->end(), result->data());
                            image = result;
                            loaded = true;
                        }
                    }
                }
            }
        } catch (...) {
            failure = "image-exception";
        }

        if (!loaded && failure)
            InvalidateKey(key, entry, failure);

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            const auto elapsed = std::chrono::steady_clock::now() - start;
            m_imageLoadMicros +=
                static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count());
            ++m_imageLoadCount;
        }
        return loaded;
    }

    bool CacheStore::StoreImage(const CacheKey& key, const CacheKey& sourceKey, const wk::RawImage& image) {
        const size_t rawSize = image.data_length();
        if (image.width() == 0 || image.height() == 0 || rawSize > std::numeric_limits<uint32_t>::max()) {
            return false;
        }

        try {
            flatbuffers::FlatBufferBuilder builder(rawSize + 512);

            const auto keyOffset = builder.CreateVector(key.data(), key.size());
            const auto sourceKeyOffset = builder.CreateVector(sourceKey.data(), sourceKey.size());
            const auto dataOffset = builder.CreateVector(image.data(), rawSize);
            const auto cachedImage = Gen::CreateCachedImage(builder,
                                                            image.width(),
                                                            image.height(),
                                                            ToCachedDepth(image.depth()),
                                                            ToCachedColorSpace(image.colorspace()),
                                                            sourceKeyOffset,
                                                            dataOffset);
            const auto blob = Gen::CreateCachedImageBlob(builder, keyOffset, cachedImage);
            Gen::FinishCachedImageBlobBuffer(builder, blob);

            const uint8_t* bytes = builder.GetBufferPointer();
            const size_t size = builder.GetSize();

            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_enabled || m_entries.count(key) != 0)
                return false;

            BlobEntry entry;
            return AppendBlobLocked(key, bytes, size, rawSize, Gen::BlobKind::Image, entry);
        } catch (...) {
            Disable("exception while storing image");
            return false;
        }
    }

    bool CacheStore::TryLoadSymbol(const CacheKey& key, Animate::Publisher::CachedSymbolPayload& payload) {
        const auto start = std::chrono::steady_clock::now();

        BlobEntry entry;
        uint64_t committedSize = 0;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_enabled)
                return false;

            auto it = m_entries.find(key);
            if (it == m_entries.end() ||
                (it->second.kind != Gen::BlobKind::Shape && it->second.kind != Gen::BlobKind::TextField &&
                 it->second.kind != Gen::BlobKind::MovieClip))
                return false;

            entry = it->second;
            committedSize = m_writeOffset;
        }

        bool loaded = false;
        const char* failure = nullptr;
        const char* readFailure = nullptr;
        uint64_t readMicros = 0;
        uint64_t hashMicros = 0;
        uint64_t decodeMicros = 0;

        // No shared state below. Checksum validation replaces the full FlatBuffers verifier:
        // the bytes are exactly what this store wrote, and hashes run in parallel now.
        try {
            std::vector<uint8_t> blob;
            const auto readStart = std::chrono::steady_clock::now();
            const bool blobRead = ReadBlob(entry, committedSize, blob, readFailure);
            readMicros = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - readStart)
                    .count());
            if (!blobRead) {
                failure = readFailure ? readFailure : "symbol-read";
            } else if (blob.size() < 8 || !Gen::CachedSymbolBlobBufferHasIdentifier(blob.data())) {
                failure = "symbol-identifier";
            } else {
                const auto hashStart = std::chrono::steady_clock::now();
                const bool checksumOk = wk::hash::fnv1a32(blob.data(), blob.size()) == entry.checksum;
                hashMicros = static_cast<uint64_t>(
                    std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - hashStart)
                        .count());
                if (!checksumOk) {
                    failure = "symbol-checksum";
                } else {
                    const auto decodeStart = std::chrono::steady_clock::now();
                    const Gen::CachedSymbolBlob* wrapper = Gen::GetCachedSymbolBlob(blob.data());
                    const Gen::CachedSymbol* cached = wrapper ? wrapper->symbol() : nullptr;
                    if (!cached || !Equals(wrapper->key(), key)) {
                        failure = "symbol-key";
                    } else {
                        Animate::Publisher::CachedSymbolPayload result;
                        if (!FromCachedSymbolKind(cached->kind(), result.type) || !cached->data() ||
                            cached->data()->empty()) {
                            failure = "symbol-data";
                        } else {
                            result.hasContent = cached->has_content();
                            result.required = cached->required();
                            result.name =
                                cached->name() ? wk::StringConverter::ToUTF16(cached->name()->str()) : std::u16string();
                            result.linkage = cached->linkage() ? cached->linkage()->str() : std::string();
                            result.data.resize(cached->data()->size());
                            std::memcpy(result.data.data(), cached->data()->data(), cached->data()->size());

                            bool requestsValid = true;
                            if (cached->requests()) {
                                result.requests.reserve(cached->requests()->size());
                                for (const Gen::CachedRequest* stored : *cached->requests()) {
                                    if (!stored)
                                        continue;

                                    Animate::Publisher::CachedSymbolRequest request;
                                    if (!FromCachedRequestKind(stored->kind(), request.kind) || !stored->key() ||
                                        stored->key()->size() != request.key.size() ||
                                        !FromCachedModifierKind(stored->modifier_kind(), request.modifier)) {
                                        failure = "symbol-request";
                                        requestsValid = false;
                                        break;
                                    }

                                    std::copy(stored->key()->begin(), stored->key()->end(), request.key.begin());

                                    request.name = stored->name() ? wk::StringConverter::ToUTF16(stored->name()->str())
                                                                  : std::u16string();
                                    request.linkage = stored->linkage() ? stored->linkage()->str() : std::string();
                                    request.required = stored->required();
                                    request.provisional = stored->provisional();
                                    result.requests.push_back(std::move(request));
                                }
                            }

                            if (requestsValid) {
                                payload = std::move(result);
                                loaded = true;
                            }
                        }
                        decodeMicros = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(
                                                                 std::chrono::steady_clock::now() - decodeStart)
                                                                 .count());
                    }
                }
            }
        } catch (...) {
            failure = "symbol-exception";
        }

        if (!loaded && failure)
            InvalidateKey(key, entry, failure);

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            const auto elapsed = std::chrono::steady_clock::now() - start;
            m_symbolLoadMicros +=
                static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count());
            m_symbolReadMicros += readMicros;
            m_symbolHashMicros += hashMicros;
            m_symbolDecodeMicros += decodeMicros;
            ++m_symbolLoadCount;
        }
        return loaded;
    }

    bool CacheStore::StoreSymbol(const CacheKey& key, const Animate::Publisher::CachedSymbolPayload& payload) {
        if (payload.data.empty())
            return false;

        try {
            flatbuffers::FlatBufferBuilder builder(payload.data.size() + 1024);

            const std::vector<uint8_t> keyBytes(key.begin(), key.end());
            const auto keyOffset = builder.CreateVector(keyBytes);
            const std::vector<uint8_t> dataBytes(reinterpret_cast<const uint8_t*>(payload.data.data()),
                                                 reinterpret_cast<const uint8_t*>(payload.data.data()) +
                                                     payload.data.size());
            const std::string name = wk::StringConverter::ToUTF8(payload.name);

            std::vector<flatbuffers::Offset<Gen::CachedRequest>> requests;
            requests.reserve(payload.requests.size());
            for (const Animate::Publisher::CachedSymbolRequest& request : payload.requests) {
                const std::vector<uint8_t> requestKey(request.key.begin(), request.key.end());
                const std::string requestName = wk::StringConverter::ToUTF8(request.name);
                requests.push_back(
                    Gen::CreateCachedRequestDirect(builder,
                                                   ToCachedRequestKind(request.kind),
                                                   &requestKey,
                                                   requestName.empty() ? nullptr : requestName.c_str(),
                                                   request.linkage.empty() ? nullptr : request.linkage.c_str(),
                                                   ToCachedModifierKind(request.modifier),
                                                   request.required,
                                                   request.provisional));
            }

            const auto symbol =
                Gen::CreateCachedSymbolDirect(builder,
                                              &keyBytes,
                                              name.empty() ? nullptr : name.c_str(),
                                              payload.linkage.empty() ? nullptr : payload.linkage.c_str(),
                                              ToCachedSymbolKind(payload.type),
                                              payload.hasContent,
                                              payload.required,
                                              &dataBytes,
                                              &requests);
            const auto blob = Gen::CreateCachedSymbolBlob(builder, keyOffset, symbol);
            Gen::FinishCachedSymbolBlobBuffer(builder, blob);

            const uint8_t* bytes = builder.GetBufferPointer();
            const size_t size = builder.GetSize();

            Gen::BlobKind kind = Gen::BlobKind::Shape;
            switch (payload.type) {
                case Animate::Publisher::WriterType::MovieClip:
                    kind = Gen::BlobKind::MovieClip;
                    break;
                case Animate::Publisher::WriterType::TextField:
                    kind = Gen::BlobKind::TextField;
                    break;
                case Animate::Publisher::WriterType::Graphic:
                default:
                    kind = Gen::BlobKind::Shape;
                    break;
            }

            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_enabled || m_entries.count(key) != 0)
                return false;

            BlobEntry entry;
            return AppendBlobLocked(key, bytes, size, payload.data.size(), kind, entry);
        } catch (...) {
            Disable("exception while storing symbol");
            return false;
        }
    }

    bool CacheStore::Commit() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_enabled || m_rawPack.size() != m_writeOffset)
            return false;

        const fs::path packTemporary = m_packPath.string() + ".tmp";
        const fs::path indexTemporary = m_indexPath.string() + ".tmp";

        try {
            flatbuffers::FlatBufferBuilder builder(4096 + m_entries.size() * 128);
            std::vector<flatbuffers::Offset<Gen::BlobEntry>> entries;
            entries.reserve(m_entries.size());

            for (const auto& pair : m_entries) {
                const std::vector<uint8_t> key(pair.first.begin(), pair.first.end());
                const BlobEntry& entry = pair.second;

                entries.push_back(Gen::CreateBlobEntryDirect(builder,
                                                             &key,
                                                             entry.kind,
                                                             entry.offset,
                                                             entry.packedSize,
                                                             entry.rawSize,
                                                             entry.checksum));
            }

            std::vector<flatbuffers::Offset<Gen::RootKey>> rootKeys;
            rootKeys.reserve(m_rootKeys.size());
            for (const CacheKey& key : m_rootKeys) {
                const std::vector<uint8_t> bytes(key.begin(), key.end());
                rootKeys.push_back(Gen::CreateRootKeyDirect(builder, &bytes));
            }

            // Persisted field name stays global_key for wire compatibility; it now holds the
            // narrowed capture policy key.
            const std::vector<uint8_t> policyKey(m_policyKey.begin(), m_policyKey.end());
            const auto index = Gen::CreateCacheIndexDirect(builder,
                                                           m_fileGuid.c_str(),
                                                           &policyKey,
                                                           m_build.c_str(),
                                                           m_generation,
                                                           m_writeOffset,
                                                           &entries,
                                                           m_rawFlaHash,
                                                           &rootKeys,
                                                           m_rootsComplete);
            Gen::FinishCacheIndexBuffer(builder, index);

            // One streaming compression call for the whole append-only image, only when
            // this publish actually stored new blobs. Offsets stay raw, so replacing the
            // frame before the index commit keeps old indexes valid.
            if (m_packDirty) {
                {
                    std::error_code ignored;
                    fs::remove(packTemporary, ignored);

                    wk::SharedMemoryStream raw(m_rawPack.data(), m_rawPack.size());
                    wk::OutputFileStream packed(packTemporary);
                    if (!packed.is_open()) {
                        DisableLocked("cannot create temporary cache.pack");
                        return false;
                    }

                    sc::ZstdCompressor::Props props;
                    props.compression_level = CacheCompressionLevel;
                    props.workers_count = CacheCompressionWorkers;
                    sc::ZstdCompressor compressor(props);
                    compressor.compress(raw, packed);
                    packed.flush();
                    if (!packed.is_open()) {
                        DisableLocked("cannot write temporary cache.pack");
                        return false;
                    }
                }

                if (!ReplaceFileWith(packTemporary, m_packPath)) {
                    std::error_code ignored;
                    fs::remove(packTemporary, ignored);
                    DisableLocked("cannot commit cache.pack");
                    return false;
                }
                m_packDirty = false;
            }

            {
                std::error_code ignored;
                fs::remove(indexTemporary, ignored);

                auto file = wk::AssetManager::Instance().write_file(indexTemporary);
                if (!file || !file->is_open()) {
                    DisableLocked("cannot create temporary cache.index");
                    return false;
                }

                file->seek(0, wk::Stream::SeekMode::Set);
                if (file->write(builder.GetBufferPointer(), builder.GetSize()) != builder.GetSize()) {
                    DisableLocked("cannot write temporary cache.index");
                    return false;
                }
            }

            if (!ReplaceFileWith(indexTemporary, m_indexPath)) {
                std::error_code ignored;
                fs::remove(indexTemporary, ignored);
                DisableLocked("cannot commit cache.index");
                return false;
            }

            m_status = std::format("committed {} entries", m_entries.size());
            ++m_generation;
            return true;
        } catch (...) {
            std::error_code ignored;
            fs::remove(packTemporary, ignored);
            fs::remove(indexTemporary, ignored);
            DisableLocked("exception while committing cache");
            return false;
        }
    }
}
