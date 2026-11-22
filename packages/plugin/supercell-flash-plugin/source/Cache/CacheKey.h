#pragma once

#include "core/hashing/hash_stream.h"
#include "core/hashing/ncrypto/xxhash.h"

#include <array>
#include <cstdint>
#include <string>

namespace sc::Adobe::Cache {
    using CacheKey = std::array<uint8_t, 8>;

    struct CacheKeyHash {
        size_t operator()(const CacheKey& key) const {
            uint64_t value = 0;
            for (uint8_t byte : key) {
                value = (value << 8) | byte;
            }
            return static_cast<size_t>(value);
        }
    };

    class CacheKeyBuilder {
    public:
        void update(const void* data, size_t length) {
            if (!data || !length)
                return;

            m_hash.update(static_cast<const uint8_t*>(data), length);
        }

        void update(const std::string& value) { update(value.data(), value.size()); }

        void update(const std::u16string& value) { update(value.data(), value.size() * sizeof(char16_t)); }

        void update(const CacheKey& value) { update(value.data(), value.size()); }

        template <typename T>
        void updateScalar(T value) {
            update(&value, sizeof(value));
        }

        CacheKey finish() const {
            CacheKey result {};
            const uint64_t digest = m_hash.digest();

            for (size_t i = 0; i < result.size(); ++i) {
                result[i] = static_cast<uint8_t>(digest >> (56 - i * 8));
            }

            return result;
        }

    private:
        wk::hash::XXHash364 m_hash;
    };

    inline CacheKey MakeCacheKey(const std::string& value) {
        CacheKeyBuilder builder;
        builder.update(value);
        return builder.finish();
    }

    inline CacheKey DeriveCacheKey(const CacheKey& base, const char* domain, const CacheKey& source) {
        CacheKeyBuilder builder;
        builder.update(base);
        builder.update(std::string(domain));
        builder.update(source);
        return builder.finish();
    }

    inline std::string ToHex(const CacheKey& key) {
        static constexpr char digits[] = "0123456789ABCDEF";

        std::string result;
        result.reserve(key.size() * 2);
        for (uint8_t byte : key) {
            result.push_back(digits[byte >> 4]);
            result.push_back(digits[byte & 0x0F]);
        }
        return result;
    }
}
