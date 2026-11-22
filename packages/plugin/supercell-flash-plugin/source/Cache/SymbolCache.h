#pragma once

#include "animate/publisher/SymbolCache.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace sc::Adobe {
    class SCWriter;
    class SCMovieclipWriter;
    class SCShapeWriter;
    class SCTextFieldWriter;

    namespace Cache {
        class CacheManager;

        class SymbolCache final : public Animate::Publisher::ISymbolCacheProvider {
        public:
            SymbolCache(CacheManager& manager, SCWriter& writer);

            bool IsEnabled() const override;

            Animate::Publisher::SymbolCacheKey KeyForSymbol(
                const Animate::Publisher::SymbolContext& symbol) const override;
            Animate::Publisher::SymbolCacheKey DeriveKey(const Animate::Publisher::SymbolCacheKey& owner,
                                                         uint32_t ordinal) const override;

            bool TryLoad(const Animate::Publisher::SymbolCacheKey& key,
                         Animate::Publisher::CachedSymbolPayload& payload) const override;
            bool Store(const Animate::Publisher::SymbolCacheKey& key,
                       const Animate::Publisher::CachedSymbolPayload& payload) override;

            bool RestoreWriter(const Animate::Publisher::CachedSymbolPayload& payload,
                               Animate::Publisher::SymbolContext& context,
                               wk::Ref<Animate::Publisher::IDisplayObjectWriter>& writer) const override;
            bool CaptureWriter(const Animate::Publisher::IDisplayObjectWriter& writer,
                               Animate::Publisher::CachedSymbolPayload& payload) const override;

        private:
            static std::vector<std::byte> StoreMovieClip(const SCMovieclipWriter& writer);
            static void RestoreMovieClip(SCMovieclipWriter& writer, const std::vector<std::byte>& data);
            static std::vector<std::byte> StoreShape(const SCShapeWriter& writer);
            static void RestoreShape(SCShapeWriter& writer, const std::vector<std::byte>& data);
            static std::vector<std::byte> StoreTextField(const SCTextFieldWriter& writer);
            static void RestoreTextField(SCTextFieldWriter& writer, const std::vector<std::byte>& data);

            CacheManager& m_manager;
            SCWriter& m_writer;
        };
    }
}
