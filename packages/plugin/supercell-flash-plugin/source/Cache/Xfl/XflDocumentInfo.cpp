#include "XflDocumentInfo.h"

#include "Cache/CacheStore.h"
#include "Cache/Generated/SymbolMeta_generated.h"
#include "animate/core/common/FCMPluginInterface.h"
#include "core/asset_manager/asset_manager.h"
#include "core/hashing/ncrypto/xxhash.h"
#include "core/io/memory_stream.h"
#include "core/memory/ref.h"
#include "zip.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <format>
#include <fstream>
#include <functional>
#include <optional>
#include <set>
#include <system_error>
#include <unordered_set>
#include <vector>

#if defined(_WIN32) || defined(_WINDOWS)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
#endif

namespace sc::Adobe::Cache {
    thread_local std::string g_lastXflError;

    namespace {
        class FlaReader {
        public:
            explicit FlaReader(const std::filesystem::path& path) :
                m_root(path),
                m_packed(path.extension() != ".xfl") {}

            ~FlaReader() {
                if (m_zip) {
                    zip_close(m_zip);
                    m_zip = nullptr;
                }
            }

            bool Open() {
                if (!m_packed) {
                    std::error_code error;
                    if (!std::filesystem::exists(m_root, error) || error)
                        return false;

                    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_root)) {
                        if (!entry.is_regular_file())
                            continue;

                        m_paths.push_back(std::filesystem::relative(entry.path(), m_root).generic_string());
                    }
                    std::sort(m_paths.begin(), m_paths.end());
                    return true;
                }

                auto& manager = wk::AssetManager::Instance();
                if (!manager.exists(m_root)) {
                    m_lastError = "FLA does not exist";
                    return false;
                }

                auto source = manager.load_file(m_root);
                if (!source || !source->is_open() || source->length() < 22) {
                    m_lastError = "FLA too small or unreadable";
                    return false;
                }

                m_bytes = wk::CreateRef<wk::MemoryStream>(source->length());
                source->seek(0);
                if (source->read(m_bytes->data(), m_bytes->length()) != m_bytes->length()) {
                    m_lastError = "cannot read FLA bytes";
                    return false;
                }

                const size_t length = m_bytes->length();
                size_t eocd = std::string::npos;
                for (size_t position = length - 22;; --position) {
                    m_bytes->seek(position);
                    char signature[4] = {};
                    m_bytes->read(signature, sizeof(signature));
                    if (std::memcmp(signature, "PK\x05\x06", sizeof(signature)) == 0) {
                        m_bytes->seek(position + 20);
                        const uint16_t commentLength = m_bytes->read_unsigned_short();
                        if (position + 22 + commentLength == length) {
                            eocd = position;
                            break;
                        }
                    }
                    if (position == 0)
                        break;
                }

                if (eocd == std::string::npos) {
                    m_lastError = "end of central directory not found";
                    return false;
                }

                m_bytes->seek(eocd + 16);
                const uint32_t cdOffset = m_bytes->read_unsigned_int();
                m_bytes->seek(eocd + 12);
                const uint32_t cdSize = m_bytes->read_unsigned_int();
                if (cdOffset <= eocd && cdSize > eocd - cdOffset) {
                    m_bytes->seek(eocd + 12);
                    m_bytes->write_unsigned_int(static_cast<uint32_t>(eocd - cdOffset));
                }

                int error = 0;
                m_zip = zip_stream_openwitherror(static_cast<const char*>(m_bytes->data()),
                                                 m_bytes->length(),
                                                 0,
                                                 'r',
                                                 &error);
                if (!m_zip) {
                    m_lastError = error ? zip_strerror(error) : "unknown zip error";
                    return false;
                }

                const size_t count = zip_entries_total(m_zip);
                for (size_t i = 0; i < count; ++i) {
                    if (zip_entry_openbyindex(m_zip, i) != 0)
                        continue;

                    if (!zip_entry_isdir(m_zip)) {
                        const char* name = zip_entry_name(m_zip);
                        if (name) {
                            std::string path(name);
                            std::replace(path.begin(), path.end(), '\\', '/');
                            m_paths.push_back(std::move(path));
                        }
                    }

                    zip_entry_close(m_zip);
                }

                std::sort(m_paths.begin(), m_paths.end());
                return true;
            }

            const std::vector<std::string>& Paths() const { return m_paths; }

            std::string LastError() const { return m_lastError; }

            bool ReadFile(const std::string& name, std::vector<uint8_t>& result) {
                if (!m_packed) {
                    auto& manager = wk::AssetManager::Instance();
                    const std::filesystem::path path = m_root / name;
                    if (!manager.exists(path))
                        return false;

                    auto file = manager.load_file(path);
                    result.resize(file->length());
                    file->seek(0);
                    return file->length() == 0 || file->read(result.data(), result.size()) == result.size();
                }

                const int openResult = zip_entry_open(m_zip, name.c_str());
                if (openResult != 0) {
                    m_lastError = std::format("zip entry open failed for {}: {}", name, openResult);
                    return false;
                }

                const size_t size = static_cast<size_t>(zip_entry_size(m_zip));
                result.resize(size);
                const ssize_t read = size == 0 ? 0 : zip_entry_noallocread(m_zip, result.data(), size);
                zip_entry_close(m_zip);
                const bool ok = size == 0 || read == static_cast<ssize_t>(size);
                if (!ok) {
                    m_lastError = std::format("zip read failed for {}: got {}, expected {}", name, read, size);
                }
                return ok;
            }

            // Read only the leading bytes of an entry
            // Symbol XML headers fit in the first 2 KB
            // the rest of the file is never decompressed
            bool ReadHeader(const std::string& name, size_t maxBytes, std::vector<uint8_t>& result) {
                if (!m_packed) {
                    auto& manager = wk::AssetManager::Instance();
                    const std::filesystem::path path = m_root / name;
                    if (!manager.exists(path))
                        return false;

                    auto file = manager.load_file(path);
                    result.resize(std::min(maxBytes, static_cast<size_t>(file->length())));
                    file->seek(0);
                    return result.empty() || file->read(result.data(), result.size()) == result.size();
                }

                if (zip_entry_open(m_zip, name.c_str()) != 0)
                    return false;

                const size_t size = std::min(maxBytes, static_cast<size_t>(zip_entry_size(m_zip)));
                result.resize(size);
                const ssize_t read = size == 0 ? 0 : zip_entry_noallocreadwithoffset(m_zip, 0, size, result.data());
                zip_entry_close(m_zip);
                return size == 0 || read == static_cast<ssize_t>(size);
            }

            // Cheap ZIP-directory/filesystem metadata fingerprint. Deliberately does not
            // decompress entry bytes: symbol XML is never re-read just to compute a key.
            uint64_t Fingerprint(const std::string& name) {
                wk::hash::XxHash hasher;
                hasher.update(reinterpret_cast<const uint8_t*>(name.data()), name.size());

                if (m_packed) {
                    if (zip_entry_open(m_zip, name.c_str()) == 0) {
                        const uint64_t size = static_cast<uint64_t>(zip_entry_size(m_zip));
                        const uint64_t crc = static_cast<uint64_t>(zip_entry_crc32(m_zip));
                        hasher.update(reinterpret_cast<const uint8_t*>(&size), sizeof(size));
                        hasher.update(reinterpret_cast<const uint8_t*>(&crc), sizeof(crc));
                        zip_entry_close(m_zip);
                    }
                } else {
                    std::error_code error;
                    const std::filesystem::path path = m_root / name;
                    const uint64_t size = static_cast<uint64_t>(std::filesystem::file_size(path, error));
                    uint64_t writeTime = 0;
                    if (!error) {
                        const auto time = std::filesystem::last_write_time(path, error);
                        if (!error)
                            writeTime = static_cast<uint64_t>(time.time_since_epoch().count());
                    }
                    hasher.update(reinterpret_cast<const uint8_t*>(&size), sizeof(size));
                    hasher.update(reinterpret_cast<const uint8_t*>(&writeTime), sizeof(writeTime));
                }

                return hasher.digest();
            }

        private:
            std::filesystem::path m_root;
            bool m_packed = true;
            wk::Ref<wk::MemoryStream> m_bytes;
            zip_t* m_zip = nullptr;
            std::vector<std::string> m_paths;
            std::string m_lastError;
        };

        void AppendUtf8(std::string& result, unsigned code) {
            if (code <= 0x7F) {
                result.push_back(static_cast<char>(code));
            } else if (code <= 0x7FF) {
                result.push_back(static_cast<char>(0xC0 | (code >> 6)));
                result.push_back(static_cast<char>(0x80 | (code & 0x3F)));
            } else if (code <= 0xFFFF) {
                result.push_back(static_cast<char>(0xE0 | (code >> 12)));
                result.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
                result.push_back(static_cast<char>(0x80 | (code & 0x3F)));
            } else {
                result.push_back(static_cast<char>(0xF0 | (code >> 18)));
                result.push_back(static_cast<char>(0x80 | ((code >> 12) & 0x3F)));
                result.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
                result.push_back(static_cast<char>(0x80 | (code & 0x3F)));
            }
        }

        std::string DecodeEntities(const std::string& value) {
            std::string result;
            result.reserve(value.size());

            for (size_t i = 0; i < value.size(); ++i) {
                if (value[i] != '&') {
                    result.push_back(value[i]);
                    continue;
                }

                const size_t end = value.find(';', i + 1);
                if (end == std::string::npos || end - i > 12) {
                    result.push_back(value[i]);
                    continue;
                }

                const std::string entity = value.substr(i + 1, end - i - 1);
                if (entity == "amp")
                    result.push_back('&');
                else if (entity == "lt")
                    result.push_back('<');
                else if (entity == "gt")
                    result.push_back('>');
                else if (entity == "quot")
                    result.push_back('"');
                else if (entity == "apos")
                    result.push_back('\'');
                else if (entity.rfind("#x", 0) == 0) {
                    const unsigned code = std::stoul(entity.substr(2), nullptr, 16);
                    AppendUtf8(result, code);
                } else if (entity.rfind('#', 0) == 0) {
                    const unsigned code = std::stoul(entity.substr(1), nullptr, 10);
                    AppendUtf8(result, code);
                } else {
                    result.append("&").append(entity).append(";");
                }

                i = end;
            }

            return result;
        }

        std::string GetAttribute(const std::string& tag, const std::string& name) {
            const std::string prefix = name + "=\"";
            const size_t start = tag.find(prefix);
            if (start == std::string::npos) {
                const std::string single = name + "='";
                const size_t singleStart = tag.find(single);
                if (singleStart == std::string::npos)
                    return {};

                const size_t singleValue = singleStart + single.size();
                const size_t singleEnd = tag.find('\'', singleValue);
                if (singleEnd == std::string::npos)
                    return {};

                return DecodeEntities(tag.substr(singleValue, singleEnd - singleValue));
            }

            const size_t value = start + prefix.size();
            const size_t end = tag.find('"', value);
            if (end == std::string::npos)
                return {};

            return DecodeEntities(tag.substr(value, end - value));
        }

        std::string NormalizeGuid(std::string value) {
            value.erase(std::remove_if(value.begin(),
                                       value.end(),
                                       [](char c) { return c == '-' || c == '{' || c == '}' || c == ' '; }),
                        value.end());

            for (char& c : value) {
                c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            }

            if (value.size() != 32)
                return {};

            for (char c : value) {
                const bool hex = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
                if (!hex)
                    return {};
            }

            return value;
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

        void AddBitmapKey(XflDocumentInfo& info, const std::u16string& name, const CacheKey& key) {
            if (!name.empty())
                info.bitmapKeys[NormalizeName(name)] = key;
        }

        void AddBitmapKeyUtf8(XflDocumentInfo& info, const std::string& name, const CacheKey& key) {
            if (!name.empty())
                info.bitmapKeysUtf8[NormalizeName(name)] = key;
        }

        bool ParseSymDependCache(const std::vector<uint8_t>& bytes,
                                 std::vector<std::string>& names,
                                 std::vector<std::vector<uint32_t>>& usage) {
            if (bytes.empty())
                return false;

            wk::MemoryStream stream(bytes.size());
            std::memcpy(stream.data(), bytes.data(), bytes.size());
            const auto remaining = [&stream]() { return stream.length() - stream.position(); };

            if (remaining() < sizeof(uint32_t) || stream.read_unsigned_int() != 103)
                return false;

            if (remaining() < 16) // SYSTEMTIME
                return false;

            stream.seek(16, wk::Stream::SeekMode::Add);

            if (remaining() < sizeof(uint32_t))
                return false;
            const uint32_t count = stream.read_unsigned_int();
            if (count > remaining() / sizeof(uint16_t))
                return false;

            names.resize(count);
            usage.resize(count);
            for (uint32_t i = 0; i < count; ++i) {
                if (remaining() < sizeof(uint16_t))
                    return false;

                const uint16_t length = stream.read_unsigned_short();
                if (remaining() < length)
                    return false;

                const char* text =
                    reinterpret_cast<const char*>(static_cast<const uint8_t*>(stream.data()) + stream.position());

                std::string name(text, length);
                // SymDepend strings are NUL-terminated inside the declared byte length.
                while (!name.empty() && name.back() == '\0')
                    name.pop_back();

                names[i] = NormalizeName(std::move(name));
                stream.seek(length, wk::Stream::SeekMode::Add);
            }

            if (remaining() < sizeof(uint32_t) || stream.read_unsigned_int() != 44478)
                return false;

            for (uint32_t i = 0; i < count; ++i) {
                if (remaining() < sizeof(uint32_t))
                    return false;

                const uint32_t userCount = stream.read_unsigned_int();
                if (userCount > remaining() / sizeof(uint32_t))
                    return false;

                usage[i].resize(userCount);
                for (uint32_t j = 0; j < userCount; ++j) {
                    if (remaining() < sizeof(uint32_t))
                        return false;

                    usage[i][j] = stream.read_unsigned_int();
                    if (usage[i][j] >= count)
                        return false;
                }
            }

            return stream.position() == stream.length();
        }

        struct PersistedSymbolMeta {
            std::string name;
            std::string linkage;
            std::string lastModified;
        };

        std::filesystem::path SymbolMetaPath(const std::string& fileGuid) {
            return CacheStore::RootDirectory() / fileGuid / "symbol-meta.index";
        }

        bool LoadSymbolMeta(const std::string& fileGuid, std::unordered_map<std::string, PersistedSymbolMeta>& result) {
            const auto path = SymbolMetaPath(fileGuid);
            auto& manager = wk::AssetManager::Instance();
            if (!manager.exists(path))
                return false;

            auto file = manager.load_file(path);
            std::vector<uint8_t> bytes(file->length());
            file->seek(0);
            if (bytes.empty() || file->read(bytes.data(), bytes.size()) != bytes.size())
                return false;

            flatbuffers::Verifier verifier(bytes.data(), bytes.size());
            if (!Gen::VerifySymbolMetaIndexBuffer(verifier))
                return false;

            const auto* index = Gen::GetSymbolMetaIndex(bytes.data());
            if (!index || index->schema_version() != 1 || !index->entries())
                return false;

            for (const auto* entry : *index->entries()) {
                if (!entry || !entry->item_id())
                    continue;

                PersistedSymbolMeta meta;
                if (entry->name())
                    meta.name = entry->name()->str();

                if (entry->linkage())
                    meta.linkage = entry->linkage()->str();

                if (entry->last_modified())
                    meta.lastModified = entry->last_modified()->str();

                result.emplace(entry->item_id()->str(), std::move(meta));
            }
            return true;
        }

        bool StoreSymbolMeta(const std::string& fileGuid,
                             const std::unordered_map<std::string, PersistedSymbolMeta>& entries) {
            flatbuffers::FlatBufferBuilder builder;

            std::vector<flatbuffers::Offset<Gen::SymbolMetaEntry>> built;
            built.reserve(entries.size());
            for (const auto& [itemId, meta] : entries) {
                auto id = builder.CreateString(itemId);
                auto name = builder.CreateString(meta.name);
                auto linkage = builder.CreateString(meta.linkage);
                auto lastModified = builder.CreateString(meta.lastModified);
                built.push_back(Gen::CreateSymbolMetaEntry(builder, id, name, linkage, lastModified));
            }

            auto vector = builder.CreateVector(built);
            auto index = Gen::CreateSymbolMetaIndex(builder, 1, vector);
            builder.Finish(index, Gen::SymbolMetaIndexIdentifier());

            const auto path = SymbolMetaPath(fileGuid);
            std::error_code error;
            std::filesystem::create_directories(path.parent_path(), error);
            if (error)
                return false;

            const auto temp = path.string() + ".tmp";
            {
                std::ofstream output(temp, std::ios::binary | std::ios::trunc);
                if (!output)
                    return false;
                output.write(reinterpret_cast<const char*>(builder.GetBufferPointer()),
                             static_cast<std::streamsize>(builder.GetSize()));
                output.flush();
                if (!output)
                    return false;
            }

#if defined(_WIN32) || defined(_WINDOWS)
            return MoveFileExA(temp.c_str(), path.string().c_str(), MOVEFILE_REPLACE_EXISTING) != FALSE;
#else
            std::filesystem::rename(temp, path, error);
            return !error;
#endif
        }

        bool ReadDocumentXml(FlaReader& file, XflDocumentInfo& result, std::string& xml) {
            const auto& paths = file.Paths();
            const auto documentPathIt = std::find(paths.begin(), paths.end(), "DOMDocument.xml");
            if (documentPathIt == paths.end()) {
                g_lastXflError = "DOMDocument.xml not found";
                return false;
            }

            std::vector<uint8_t> documentBytes;
            if (!file.ReadFile(*documentPathIt, documentBytes)) {
                g_lastXflError = "cannot read DOMDocument.xml: " + file.LastError();
                return false;
            }

            if (documentBytes.empty()) {
                g_lastXflError = "DOMDocument.xml is empty";
                return false;
            }

            xml.assign(reinterpret_cast<const char*>(documentBytes.data()), documentBytes.size());

            const size_t documentTagStart = xml.find("<DOMDocument");
            if (documentTagStart == std::string::npos)
                return false;

            const size_t documentTagEnd = xml.find('>', documentTagStart);
            if (documentTagEnd == std::string::npos)
                return false;

            const std::string documentTag = xml.substr(documentTagStart, documentTagEnd - documentTagStart + 1);
            result.fileGuid = NormalizeGuid(GetAttribute(documentTag, "fileGUID"));
            if (result.fileGuid.empty()) {
                g_lastXflError = "fileGUID missing or invalid";
                return false;
            }

            return true;
        }

        std::string ReadSymbolLinkage(FlaReader& file, const std::string& href) {
            const std::string entry = "LIBRARY/" + href + ".xml";
            std::vector<uint8_t> headerBytes;
            if (!file.ReadHeader(entry, 4096, headerBytes) || headerBytes.empty())
                return {};

            const std::string text(reinterpret_cast<const char*>(headerBytes.data()), headerBytes.size());
            const size_t itemStart = text.find("<DOMSymbolItem");
            if (itemStart == std::string::npos)
                return {};

            const size_t itemEnd = text.find('>', itemStart);
            if (itemEnd == std::string::npos)
                return {};

            return GetAttribute(text.substr(itemStart, itemEnd - itemStart + 1), "linkageClassName");
        }

        void ParseSymbols(FlaReader& file, const std::string& xml, XflDocumentInfo& result) {
            // Linkage lives only in symbol XML headers. Persist it by stable itemID so an
            // unchanged symbol never reopens its XML on later publishes.
            std::unordered_map<std::string, PersistedSymbolMeta> persisted;
            const bool validMeta = LoadSymbolMeta(result.fileGuid, persisted);
            bool metaDirty = false;

            const size_t symbolsStart = xml.find("<symbols");
            const size_t symbolsEnd =
                symbolsStart == std::string::npos ? std::string::npos : xml.find("</symbols>", symbolsStart);
            if (symbolsStart == std::string::npos || symbolsEnd == std::string::npos)
                return;

            const std::string section = xml.substr(symbolsStart, symbolsEnd - symbolsStart);
            size_t includePosition = 0;
            while ((includePosition = section.find("<Include", includePosition)) != std::string::npos) {
                const size_t includeEnd = section.find('>', includePosition);
                if (includeEnd == std::string::npos)
                    break;

                const std::string tag = section.substr(includePosition, includeEnd - includePosition + 1);
                includePosition = includeEnd + 1;

                std::string href = NormalizeName(DecodeEntities(GetAttribute(tag, "href")));
                if (href.empty())
                    continue;

                const size_t libraryPrefix = std::string("LIBRARY/").size();
                if (href.starts_with("LIBRARY/"))
                    href.erase(0, libraryPrefix);

                if (href.ends_with(".xml"))
                    href.resize(href.size() - 4);

                if (href.empty())
                    continue;

                XflSymbolInfo info;
                info.itemId = GetAttribute(tag, "itemID");
                info.lastModified = GetAttribute(tag, "lastModified");

                std::string linkage;
                auto cached = persisted.find(info.itemId);
                if (validMeta && cached != persisted.end() && cached->second.lastModified == info.lastModified &&
                    cached->second.name == href) {
                    linkage = cached->second.linkage;
                } else if (!info.itemId.empty()) {
                    // First sight, edit, or rename: read only the symbol header.
                    linkage = ReadSymbolLinkage(file, href);

                    PersistedSymbolMeta updated;
                    updated.name = href;
                    updated.linkage = linkage;
                    updated.lastModified = info.lastModified;
                    persisted[info.itemId] = std::move(updated);
                    metaDirty = true;
                }

                if (!linkage.empty())
                    result.linkages[href] = linkage;

                result.symbols[href] = std::move(info);
            }

            if (metaDirty)
                StoreSymbolMeta(result.fileGuid, persisted);
        }

        uint64_t ComputeDocumentHash(FlaReader& file, const std::vector<std::string>& paths) {
            // Cheap name/size/CRC fingerprint
            wk::hash::XxHash hasher;
            for (const auto& entry : paths) {
                if (!entry.starts_with("LIBRARY") && !entry.starts_with("bin"))
                    continue;

                hasher.update(reinterpret_cast<const uint8_t*>(entry.data()), entry.size());
                const uint64_t fingerprint = file.Fingerprint(entry);
                hasher.update(reinterpret_cast<const uint8_t*>(&fingerprint), sizeof(fingerprint));
            }
            return hasher.digest();
        }

        void ParseBitmapKeys(const std::string& xml, XflDocumentInfo& result) {
            size_t position = 0;
            while ((position = xml.find("<DOMBitmapItem", position)) != std::string::npos) {
                const size_t end = xml.find('>', position);
                if (end == std::string::npos)
                    break;

                const std::string tag = xml.substr(position, end - position + 1);
                position = end + 1;

                CacheKeyBuilder builder;

                // Authoring metadata already carries CRC/import date/size. Reading bitmap
                // bytes here is unnecessary and would make every publish decompress the atlas.
                static constexpr const char* attributes[] = {"name",
                                                             "itemID",
                                                             "href",
                                                             "bitmapDataHRef",
                                                             "sourceLastImported",
                                                             "externalFileCRC32",
                                                             "externalFileSize",
                                                             "allowSmoothing",
                                                             "compressionType",
                                                             "frameRight",
                                                             "frameBottom"};

                for (const char* attribute : attributes) {
                    builder.update(std::string(attribute));
                    builder.update(GetAttribute(tag, attribute));
                }

                const CacheKey sourceKey = builder.finish();
                const std::string href = GetAttribute(tag, "href");
                const std::string name = GetAttribute(tag, "name");

                AddBitmapKey(result, FCM::Locale::ToUtf16(name), sourceKey);
                AddBitmapKeyUtf8(result, name, sourceKey);
                if (!href.empty() && href != name) {
                    AddBitmapKey(result, FCM::Locale::ToUtf16(href), sourceKey);
                    AddBitmapKeyUtf8(result, href, sourceKey);
                }
            }
        }

        using SymbolDependencies = std::unordered_map<std::string, std::vector<std::string>>;

        SymbolDependencies ReadDependencies(FlaReader& file, const std::vector<std::string>& paths) {
            // Animate's own dependency registry. users[i] lists the items that USE item i,
            // so invert it to get direct dependencies for every library item. No symbol XML
            // is opened here.
            SymbolDependencies dependencies;
            auto registry = std::find(paths.begin(), paths.end(), "bin/SymDepend.cache");
            if (registry == paths.end())
                return dependencies;

            std::vector<uint8_t> bytes;
            if (!file.ReadFile(*registry, bytes))
                return dependencies;

            std::vector<std::string> registryNames;
            std::vector<std::vector<uint32_t>> users;
            if (!ParseSymDependCache(bytes, registryNames, users))
                return dependencies;

            for (size_t i = 0; i < registryNames.size(); ++i) {
                for (uint32_t user : users[i]) {
                    if (user < registryNames.size())
                        dependencies[registryNames[user]].push_back(registryNames[i]);
                }
            }

            for (auto& [name, deps] : dependencies) {
                (void) name;
                std::sort(deps.begin(), deps.end());
                deps.erase(std::unique(deps.begin(), deps.end()), deps.end());
            }

            return dependencies;
        }

        struct SymbolHashBuilder {
            XflDocumentInfo& result;
            const SymbolDependencies& dependencies;
            std::unordered_map<std::string, CacheKey> computed;
            std::unordered_set<std::string> visiting;

            // Content hash per authored symbol: own authoring metadata plus the sorted
            // content keys of its dependency subtree. Name/path/linkage are NOT mixed in,
            // so a plain rename keeps every key stable. Cycles broken by the visiting set.
            std::optional<CacheKey> Compute(const std::string& name) {
                if (auto found = computed.find(name); found != computed.end())
                    return found->second;
                if (!visiting.insert(name).second)
                    return std::nullopt;

                CacheKeyBuilder builder;
                if (auto symbol = result.symbols.find(name); symbol != result.symbols.end()) {
                    builder.update(symbol->second.itemId);
                    builder.update(symbol->second.lastModified);
                } else {
                    builder.update("external");
                }

                std::vector<CacheKey> childKeys;
                if (auto deps = dependencies.find(name); deps != dependencies.end()) {
                    for (const std::string& reference : deps->second) {
                        if (auto bitmap = result.bitmapKeysUtf8.find(reference);
                            bitmap != result.bitmapKeysUtf8.end()) {
                            childKeys.push_back(bitmap->second);
                            continue;
                        }
                        if (result.symbols.find(reference) == result.symbols.end())
                            continue;

                        if (auto childHash = Compute(reference); childHash)
                            childKeys.push_back(*childHash);
                    }
                }

                std::sort(childKeys.begin(), childKeys.end());
                for (const CacheKey& child : childKeys) {
                    builder.update("dep");
                    builder.update(child);
                }

                visiting.erase(name);
                computed[name] = builder.finish();
                return computed[name];
            }
        };

        void ComputeSymbolHashes(XflDocumentInfo& result, const SymbolDependencies& dependencies) {
            SymbolHashBuilder builder {result, dependencies};

            std::vector<std::string> names;
            names.reserve(result.symbols.size());
            for (const auto& [name, symbol] : result.symbols) {
                (void) symbol;
                names.push_back(name);
            }
            std::sort(names.begin(), names.end());

            for (const std::string& name : names) {
                auto hash = builder.Compute(name);
                if (!hash)
                    continue;

                result.symbolHashes[name] = *hash;
                if (auto linkage = result.linkages.find(name);
                    linkage != result.linkages.end() && !linkage->second.empty()) {
                    result.symbolHashesByLinkage[linkage->second] = *hash;
                }
            }
        }
    }

    bool XflDocumentReader::TryRead(const std::filesystem::path& documentPath, XflDocumentInfo& result) {
        g_lastXflError.clear();

        try {
            FlaReader file(documentPath);
            if (!file.Open()) {
                g_lastXflError = "cannot open FLA: " + file.LastError();
                return false;
            }

            std::string xml;
            if (!ReadDocumentXml(file, result, xml))
                return false;

            ParseSymbols(file, xml, result);
            result.hash = ComputeDocumentHash(file, file.Paths());
            ParseBitmapKeys(xml, result);

            const SymbolDependencies dependencies = ReadDependencies(file, file.Paths());
            ComputeSymbolHashes(result, dependencies);
            return true;
        } catch (const std::exception& exception) {
            g_lastXflError = exception.what();
            return false;
        } catch (...) {
            g_lastXflError = "unknown XFL read error";
            return false;
        }
    }

    std::string XflDocumentReader::LastError() {
        return g_lastXflError;
    }
}
