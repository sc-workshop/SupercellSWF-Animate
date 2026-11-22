#pragma once

#include "AnimateWriter.h"
#include "AtlasWriter.h"
#include "Module/Module.h"
#include "core/memory/ref.h"
#include "flash/flash.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <thread>

namespace sc::Adobe {
    using namespace Animate::Publisher;

    namespace Cache {
        class CacheManager;
        class SymbolCache;
    }

    class SCWriter : public SharedWriter {
    public:
        friend AtlasWriter;
        using GraphicGroup = StaticElementsGroup;

    public:
        explicit SCWriter(std::shared_ptr<Cache::CacheManager> cache = {});
        virtual ~SCWriter();

    public:
        virtual void SetDocument(const std::vector<SymbolContext>& symbols);
        virtual wk::Ref<SharedMovieclipWriter> AddMovieclip(SymbolContext& symbol);
        virtual wk::Ref<SharedShapeWriter> AddShape(SymbolContext& symbol);
        virtual wk::Ref<SharedTextFieldWriter> AddTextField(SymbolContext& symbol);

        virtual void AddModifier(ResourceReference ref, MaskedLayerState type);

        virtual void Finalize();

        virtual Animate::Publisher::ISymbolCacheProvider* SymbolCache() const override;

    public:
        static fs::path generateTempName();

        // Paths to temp sprite files. Vector fills keep SkData mappings alive while
        // painting, so they need a different file from bitmap extraction.
        const fs::path sprite_temp_path = generateTempName().concat(".png");
        const fs::path pattern_temp_path = generateTempName().concat(".png");

        // kokoro
        flash::SupercellSWF swf;

        // Unknown flag for movieclip
        bool movieclip_flag = true;

    public:
        void IncrementSymbolsProcessed();
        wk::RawImageRef GetBitmap(const BitmapElement& item);
        void AddGraphicGroup(const GraphicGroup& group);

        /// @brief Selects the document whose cache namespace is used for bitmap lookups.
        void SetCacheDocument(const fs::path& documentPath);

        /// @brief Reports whether the calling thread is the one that started publishing.
        ///
        /// Kept for call sites that need to distinguish serial commit work from parallel prepare
        /// work. Progress reporting itself is thread-safe and needs no guard.
        bool IsPublishThread() const;

        /// @brief Guards the temporary raster image file, which is shared by all worker threads.
        std::mutex& ImageMutex() { return m_image_mutex; }

    public:
        /// @brief Looks up a matrix in one of the global transformation banks.
        ///
        /// The banks grow to tens of thousands of entries, so the upstream scan is expensive and
        /// commit performs one lookup per frame element per candidate bank. Each bank therefore
        /// keeps an index of its entries (see SyncBankLookup), which turns the lookup into a few
        /// hash probes followed by the exact comparison. Results are identical to the upstream
        /// scan by construction: candidates are always verified with Matrix2D::operator== and the
        /// lowest matching index wins, which is what the linear scan returns.
        bool FindMatrixIndex(size_t bank_index,
                             const flash::MatrixBank& bank,
                             const flash::Matrix2D& matrix,
                             uint16_t& index);

        /// @brief Same as FindMatrixIndex, for color transforms.
        bool FindColorTransformIndex(size_t bank_index,
                                     const flash::MatrixBank& bank,
                                     const flash::ColorTransform& color,
                                     uint16_t& index);

    public:
        uint16_t LoadExternal(fs::path path);
        void SetupTextureSettings();

    private:
        // Storage for Atlas Generator guys.
        // Represents swf shapes and must have the same size as shapes vector
        std::vector<GraphicGroup> m_graphic_groups;

        // Name / Image
        std::unordered_map<std::u16string, wk::RawImageRef> m_cached_images;

        // Total symbols exported in the commit phase
        size_t m_symbols_processed = 0;

        // Thread that constructed the writer, and therefore owns publishing and the window
        const std::thread::id m_owner_thread = std::this_thread::get_id();

        // Serializes access to the shared temporary raster image file
        std::mutex m_image_mutex;

        // Persistent cache, shared with the publisher for index commit.
        std::shared_ptr<Cache::CacheManager> m_cache;
        std::unique_ptr<Cache::SymbolCache> m_symbolCache;

        /// @brief Lookup index over one global transformation bank, built as the bank grows.
        struct BankLookup {
            // Cell key -> indices of the bank entries that fall into that cell.
            std::unordered_map<uint64_t, std::vector<uint16_t>> matrix_cells;
            // Packed color transform -> index of its first occurrence.
            std::unordered_map<uint64_t, uint16_t> color_indices;
            // Number of bank entries already inserted (banks are append-only).
            size_t synced_matrices = 0;
            size_t synced_colors = 0;
        };

        /// @brief Brings the lookup index of a bank up to date with its contents.
        void SyncBankLookup(size_t bank_index, const flash::MatrixBank& bank);

        // Indexed by the position of the bank in flash::SupercellSWF::matrixBanks
        std::vector<BankLookup> m_bank_lookups;

        AtlasWriter m_atlas;
    };
}
