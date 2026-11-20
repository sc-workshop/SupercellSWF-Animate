#include "Writer.h"

#include "Module/SCPluginException.h"
#include "MovieclipWriter.h"
#include "Reassemble/Object.hpp"
#include "ShapeWriter.h"
#include "TextFieldWriter.h"
#include "core/exception/exception.h"
#include "core/io/endian.h"
#include "core/io/file_stream.h"
#include "core/stb/stb.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fmt/format.h>
#include <system_error>

#if defined(_WIN32) || defined(_WINDOWS)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
#elif defined(__APPLE__)
    #include <unistd.h>
#endif

using namespace Animate::Publisher;

namespace {
    inline bool FloatEqual(float a, float b) {
        return std::fabs(a - b) <= ((std::fabs(a) < std::fabs(b) ? std::fabs(b) : std::fabs(a)) * 0.001f);
    }

    inline uint64_t PackColorTransform(const sc::flash::ColorTransform& color) {
        return (uint64_t) color.multiply.r | ((uint64_t) color.multiply.g << 8) | ((uint64_t) color.multiply.b << 16) |
               ((uint64_t) color.alpha << 24) | ((uint64_t) color.add.r << 32) | ((uint64_t) color.add.g << 40) |
               ((uint64_t) color.add.b << 48);
    }

    /// @brief Total-order integer image of a float: monotone in the value, one unit per ULP.
    inline uint32_t SortableFloat(float value) {
        uint32_t bits;
        std::memcpy(&bits, &value, sizeof(bits));
        return (bits & 0x80000000u) != 0 ? ~bits : (bits | 0x80000000u);
    }

    // Width of an index cell, in ULP units. Matrix2D::operator== treats two components as equal
    // when they differ by at most 0.1% of the larger magnitude, which is at most 16777 ULP units
    // (a 23-bit mantissa step is 2^-23 relative, and the bound doubles across a power of two).
    // A cell is much wider than the tolerance box, so the box never overlaps more than two cells
    // per component: probing the overlapping cells cannot miss a matching entry.
    constexpr uint32_t CELL_SHIFT = 18;
    constexpr int64_t CELL_HALF_WIDTH = 16777;

    inline uint64_t CellKey(const uint32_t (&cells)[6]) {
        uint64_t key = 1469598103934665603ull;
        for (uint32_t cell : cells) {
            key = (key ^ cell) * 1099511628211ull;
        }
        return key;
    }

    /// @brief Fills the cell coordinates of a matrix.
    inline void MatrixCells(const sc::flash::Matrix2D& matrix, uint32_t (&cells)[6]) {
        const float values[6] = {matrix.a, matrix.b, matrix.c, matrix.d, matrix.tx, matrix.ty};
        for (int i = 0; i < 6; i++) {
            cells[i] = SortableFloat(values[i]) >> CELL_SHIFT;
        }
    }
} // namespace

namespace sc::Adobe {
    SCWriter::SCWriter() :
        m_atlas(*this) {
    }

    SCWriter::~SCWriter() {
        // Destructors are implicitly noexcept. Use the error_code overload: a transient
        // cleanup failure (locked/missing file, bad temp path) must not terminate the host.
        std::error_code ignored;
        fs::remove(sprite_temp_path, ignored);
    }

    void SCWriter::SetDocument(const std::vector<SymbolContext>& symbols) {
        SCPlugin& context = SCPlugin::Instance();
        ExportProgress* progress = context.Progress();

        progress->SetCounts(0, symbols.size());
        progress->SetDetail(context.locale.GetU8String("TID_BAR_LABEL_LIBRARY_ITEMS"));
    }

    wk::Ref<SharedMovieclipWriter> SCWriter::AddMovieclip(SymbolContext& symbol) {
        return wk::CreateRef<SCMovieclipWriter>(*this, symbol);
    }

    wk::Ref<SharedShapeWriter> SCWriter::AddShape(SymbolContext& symbol) {
        return wk::CreateRef<SCShapeWriter>(*this, symbol);
    }

    void SCWriter::AddModifier(ResourceReference ref, MaskedLayerState type) {
        flash::MovieClipModifier& modifier = swf.movieclip_modifiers.emplace_back();
        modifier.id = ref.GetId();

        switch (type) {
            case MaskedLayerState::MASK_LAYER:
                modifier.type = flash::MovieClipModifier::Type::Mask;
                break;
            case MaskedLayerState::MASKED_LAYERS:
                modifier.type = flash::MovieClipModifier::Type::Masked;
                break;
            case MaskedLayerState::MASKED_LAYERS_END:
                modifier.type = flash::MovieClipModifier::Type::Unmasked;
                break;
            default:
                break;
        }
    }

    wk::Ref<SharedTextFieldWriter> SCWriter::AddTextField(Animate::Publisher::SymbolContext& symbol) {
        return wk::CreateRef<SCTextFieldWriter>(*this, symbol);
    }

    uint16_t SCWriter::LoadExternal(fs::path path) {
        using namespace Animate::DOM;
        SCConfig& config = SCPlugin::Publisher::ActiveConfig();
        swf.current_file = path;

        bool isSc2 = false;
        if (config.autoProperties) {
            wk::InputFileStream file(path);
            uint32_t scVersion = flash::SupercellSWF::GetVersion(file);

            if (scVersion >= 5) {
                isSc2 = true;
                if (scVersion == 6) {
                    file.read_unsigned_short(); // always 0
                }

                config.type = SCConfig::SWFType::SC2;
                swf.load_sc2(file);

                config.useShortFrames = swf.sc2_compile_settings.use_short_frames;
                config.lowPrecisionMatrices = swf.sc2_compile_settings.use_half_precision_matrices;
            } else {
                // Yeah i need to paste here whole decompressor function because of this feature
                {
                    file.seek(0);

                    // Reading file magic
                    std::uint16_t magic = file.read_unsigned_short(wk::Endian::Big);
                    if (magic != flash::SC_MAGIC) {
                        throw wk::Exception("Bad compressed file magic!");
                    }

                    // Checking compression version
                    std::uint32_t version = file.read_unsigned_int(wk::Endian::Big);
                    if (version == 4) {
                        version = file.read_unsigned_int(wk::Endian::Big);
                    }

                    std::uint32_t hash_length = file.read_unsigned_int(wk::Endian::Big);
                    file.seek(hash_length, wk::Stream::SeekMode::Add); // Skip hash

                    // Version 3 is zstandard
                    if (version == 3) {
                        config.compression = flash::Signature::Zstandard;
                        ZstdDecompressor context;
                        context.decompress(file, swf.stream);
                    } else if (version == 1) // Version 1 is lzma or lzham
                    {
                        std::uint8_t header[lzma::PROPS_SIZE];
                        file.read(header, lzma::PROPS_SIZE);
                        std::uint32_t unpacked_length = file.read_unsigned_int();

                        // Check SCLZ magic
                        if (*(std::uint32_t*) header == flash::SC_LZHAM_MAGIC) {
                            config.compression = flash::Signature::Lzham;

                            LzhamDecompressor::Props props;
                            props.dict_size_log2 = header[4];
                            props.unpacked_length = unpacked_length;

                            LzhamDecompressor context(props);
                            context.decompress(file, swf.stream);
                        } else {
                            config.compression = flash::Signature::Lzma;

                            LzmaDecompressor context(header, unpacked_length);
                            context.decompress(file, swf.stream);
                        }
                    } else {
                        throw wk::Exception("Unknown SC1 version");
                    }
                }

                config.hasExternalTexture = swf.load_sc1(false);
                if (config.hasExternalTexture)
                    swf.load_external_texture();
                swf.stream.clear();
            }

            fs::path basename = path.stem();
            fs::path dirname = path.parent_path();

            config.hasExternalTextureFile = swf.use_external_textures;
            config.hasLowresTexture = swf.use_low_resolution;
            config.hasMultiresTexture = swf.use_multi_resolution;
            config.lowResolutionSuffix = swf.low_resolution_suffix.string();
            config.multiResolutionSuffix = swf.multi_resolution_suffix.string();

            auto lowresPath = dirname / fs::path(basename).concat(swf.low_resolution_suffix.string()).concat("_tex.sc");
            config.generateLowresTexture = fs::exists(lowresPath);

            if (!swf.textures.empty()) {
                const auto& texture = swf.textures.front();
                config.textureEncoding = texture.encoding();
                if (texture.encoding() == flash::SWFTexture::TextureEncoding::SupercellTexture) {
                    const auto image = std::static_pointer_cast<texture::SupercellTexture>(texture.image());
                    config.generateStreamingTexture = !image->proxy_textures.empty();
                }
            }

            for (auto& movieclip : swf.movieclips) {
                if (!movieclip.custom_properties.empty()) {
                    config.writeCustomProperties = true;
                    break;
                }
            }

            movieclip_flag = false;
            for (auto& movieclip : swf.movieclips) {
                if (movieclip.unknown_flag) {
                    movieclip_flag = true;
                    break;
                }
            }
        } else {
            {
                wk::InputFileStream file(path);
                isSc2 = flash::SupercellSWF::IsSC2(file);
            }

            swf.load(path);
        }

        if (isSc2) {
            for (auto& shape : swf.shapes) {
                for (auto& command : shape.commands) {
                    command.sort_advanced_vertices();
                }
            }
        }

        m_atlas.texture_offset = swf.textures.size();
        uint16_t idOffset = 0;
        {
            for (flash::MovieClip& object : swf.movieclips) {
                if (object.id > idOffset) {
                    idOffset = object.id;
                }
            }

            for (flash::MovieClipModifier& object : swf.movieclip_modifiers) {
                if (object.id > idOffset) {
                    idOffset = object.id;
                }
            }

            for (flash::TextField& object : swf.textfields) {
                if (object.id > idOffset) {
                    idOffset = object.id;
                }
            }

            for (flash::Shape& object : swf.shapes) {
                if (object.id > idOffset) {
                    idOffset = object.id;
                }
            }
        }

        return ++idOffset;
    }

    void SCWriter::SetupTextureSettings() {
        SCConfig& config = SCPlugin::Publisher::ActiveConfig();

        // limit encoding type only to those supported by SC1
        if (config.type == SCConfig::SWFType::SC1 && !config.hasExternalTextureFile) {
            if (config.textureEncoding == flash::SWFTexture::TextureEncoding::SupercellTexture) {
                config.textureEncoding = flash::SWFTexture::TextureEncoding::KhronosTexture; // Fallback to zktx
                config.compressExternalTextureFile = true;
                config.hasExternalTextureFile = true;
            }
        }

        // Raw textures can be stored only inside texture files in SC1 and inside SC2
        if (config.hasExternalTextureFile && config.textureEncoding == flash::SWFTexture::TextureEncoding::Raw) {
            swf.use_external_textures = false;
        } else {
            swf.use_external_textures = config.hasExternalTextureFile;
            swf.compress_external_textures = config.compressExternalTextureFile;
        }

        swf.use_multi_resolution = config.hasMultiresTexture;
        swf.multi_resolution_suffix = flash::SWFString(config.multiResolutionSuffix);
        swf.low_resolution_suffix = flash::SWFString(config.lowResolutionSuffix);
        swf.use_external_texture = config.hasExternalTexture;
        swf.use_low_resolution = config.hasLowresTexture;
        swf.use_texture_streaming = config.generateStreamingTexture;
    }

    bool SCWriter::FindMatrixIndex(size_t bank_index,
                                   const flash::MatrixBank& bank,
                                   const flash::Matrix2D& matrix,
                                   uint16_t& index) {
        if (FloatEqual(matrix.a, 1.0f) && FloatEqual(matrix.b, 0.f) && FloatEqual(matrix.c, 0.f) &&
            FloatEqual(matrix.d, 1.0f) && FloatEqual(matrix.tx, 0.f) && FloatEqual(matrix.ty, 0.f)) {
            index = 0xFFFF;
            return true;
        }

        SyncBankLookup(bank_index, bank);

        // Tolerance box of the query in cell space
        // clamped to the valid range of the encoding
        uint32_t low[6];
        uint32_t high[6];
        int counts[6];
        const uint32_t keys[6] = {SortableFloat(matrix.a),
                                  SortableFloat(matrix.b),
                                  SortableFloat(matrix.c),
                                  SortableFloat(matrix.d),
                                  SortableFloat(matrix.tx),
                                  SortableFloat(matrix.ty)};
        for (int i = 0; i < 6; i++) {
            int64_t begin = (int64_t) keys[i] - CELL_HALF_WIDTH;
            int64_t end = (int64_t) keys[i] + CELL_HALF_WIDTH;
            if (begin < 0)
                begin = 0;
            if (end > 0xFFFFFFFFll)
                end = 0xFFFFFFFFll;

            low[i] = (uint32_t) ((uint64_t) begin >> CELL_SHIFT);
            high[i] = (uint32_t) ((uint64_t) end >> CELL_SHIFT);
            counts[i] = (int) (high[i] - low[i] + 1);
        }

        uint16_t best = 0xFFFF;
        auto& lookup = m_bank_lookups[bank_index].matrix_cells;

        int offsets[6] = {0, 0, 0, 0, 0, 0};
        for (;;) {
            uint32_t probe[6];
            for (int i = 0; i < 6; i++) {
                probe[i] = low[i] + (uint32_t) offsets[i];
            }

            auto cell = lookup.find(CellKey(probe));
            if (cell != lookup.end()) {
                for (uint16_t candidate : cell->second) {
                    if (candidate < best && bank.matrices[candidate] == matrix) {
                        best = candidate;
                    }
                }
            }

            int dimension = 5;
            while (dimension >= 0 && ++offsets[dimension] >= counts[dimension]) {
                offsets[dimension] = 0;
                dimension--;
            }
            if (dimension < 0)
                break;
        }

        if (best == 0xFFFF)
            return false;

        index = best;
        return true;
    }

    bool SCWriter::FindColorTransformIndex(size_t bank_index,
                                           const flash::MatrixBank& bank,
                                           const flash::ColorTransform& color,
                                           uint16_t& index) {
        if (color.alpha == 0xFF && color.add.r == 0 && color.add.g == 0 && color.add.b == 0 &&
            color.multiply.r == 0xFF && color.multiply.g == 0xFF && color.multiply.b == 0xFF) {
            index = 0xFFFF;
            return true;
        }

        SyncBankLookup(bank_index, bank);

        auto& lookup = m_bank_lookups[bank_index].color_indices;
        auto entry = lookup.find(PackColorTransform(color));
        if (entry == lookup.end())
            return false;

        index = entry->second;
        return true;
    }

    void SCWriter::SyncBankLookup(size_t bank_index, const flash::MatrixBank& bank) {
        if (m_bank_lookups.size() <= bank_index) {
            m_bank_lookups.resize(bank_index + 1);
        }

        BankLookup& lookup = m_bank_lookups[bank_index];

        // Both banks are append-only, so only the tail has to be inserted.
        for (size_t i = lookup.synced_matrices; i < bank.matrices.size(); i++) {
            uint32_t cells[6];
            MatrixCells(bank.matrices[i], cells);
            lookup.matrix_cells[CellKey(cells)].push_back((uint16_t) i);
        }
        lookup.synced_matrices = bank.matrices.size();

        for (size_t i = lookup.synced_colors; i < bank.color_transforms.size(); i++) {
            // Keep the first occurrence, which is what the linear scan returns.
            lookup.color_indices.emplace(PackColorTransform(bank.color_transforms[i]), (uint16_t) i);
        }
        lookup.synced_colors = bank.color_transforms.size();
    }

    void SCWriter::Finalize() {
        const SCConfig& config = SCPlugin::Publisher::ActiveConfig();
        SCPlugin& context = SCPlugin::Instance();
        ExportProgress* progress = context.Progress();

        if (swf.exports.empty())
            throw SCPluginException("TID_SWF_NO_EXPORTS", config.exportToExternalPath.string());

        progress->BeginPhase(ExportPhase::Atlas, 0, context.locale.GetU8String("TID_STATUS_TEXTURE_SAVE"));
        ThrowIfCancelled(progress);

        if (config.exportToExternal)
            flash::remove_unused(swf);

        SetupTextureSettings();
        m_atlas.WriteAtlas();
        ThrowIfCancelled(progress);

        swf.use_precision_matrix = config.hasPrecisionMatrices;
        swf.save_custom_property = config.writeCustomProperties;

        // SC2 settings
        {
            flash::Sc2CompileSettings& sc2 = swf.sc2_compile_settings;
            sc2.use_half_precision_matrices = config.lowPrecisionMatrices;
            sc2.use_short_frames = config.useShortFrames;
        }

        fs::path filepath = fs::path(config.outputFilepath).replace_extension("sc");
        std::string basename = filepath.filename().string();

        progress->BeginPhase(ExportPhase::Saving, 0, context.locale.GetU8String("TID_FILE_SAVE", basename));
        ThrowIfCancelled(progress);

        // SupercellSWF writes the .sc plus any external textures directly from the supplied path.
        // Output files are written in place; user is responsible for cleaning them if publish is cancelled.
        switch (config.type) {
            case SCConfig::SWFType::SC1:
                swf.save(filepath, config.compression, config.generateLowresTexture);
                break;
            case SCConfig::SWFType::SC2:
                swf.save_sc2(filepath);
                break;
            default:
                break;
        }

        ThrowIfCancelled(progress);
    }

    fs::path SCWriter::generateTempName() {
#if defined(_WIN32) || defined(_WINDOWS)
        wchar_t directory[MAX_PATH] = {};
        UINT directoryLength = GetTempPathW(MAX_PATH, directory);
        if (directoryLength != 0 && directoryLength < MAX_PATH) {
            wchar_t name[MAX_PATH] = {};
            if (GetTempFileNameW(directory, L"swf", 0, name) != 0) {
                // GetTempFileName creates an empty file; bitmap export expects to create it.
                std::error_code ignored;
                fs::remove(fs::path(name), ignored);
                return fs::path(name);
            }
        }
        return fs::temp_directory_path() / "swfSprite.png";
#elif defined(__APPLE__)
        char name[] = "/tmp/swfSpriteXXXXXX";
        int descriptor = mkstemp(name);
        if (descriptor >= 0)
            close(descriptor);

        // mkstemp creates an empty file; bitmap export expects to create it.
        std::error_code ignored;
        fs::remove(fs::path(name), ignored);
        return fs::path(name);
#else
        return fs::temp_directory_path() / "swfSprite.png";
#endif
    }

    void SCWriter::IncrementSymbolsProcessed() {
        SCPlugin& context = SCPlugin::Instance();
        ExportProgress* progress = context.Progress();

        ++m_symbols_processed;
        progress->Advance();
        ThrowIfCancelled(progress);
    }

    bool SCWriter::IsPublishThread() const {
        return std::this_thread::get_id() == m_owner_thread;
    }

    wk::RawImageRef SCWriter::GetBitmap(const BitmapElement& item) {
        const std::u16string& name = item.Name();

        // Library extraction runs in parallel, but the sprite cache and the temporary image file
        // are shared. Keep both under one lock.
        std::lock_guard<std::mutex> lock(m_image_mutex);

        auto it = m_cached_images.find(name);
        if (it != m_cached_images.end()) {
            return it->second;
        }

        item.ExportImage(sprite_temp_path);

        wk::RawImageRef image;
        wk::InputFileStream file(sprite_temp_path);
        wk::stb::load_image(file, image);
        m_cached_images[name] = image;

        return image;
    }

    void SCWriter::AddGraphicGroup(const GraphicGroup& group) {
        m_graphic_groups.push_back(group);
    }
}
