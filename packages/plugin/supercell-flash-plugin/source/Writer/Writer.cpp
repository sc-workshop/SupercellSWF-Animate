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

using namespace Animate::Publisher;

namespace sc::Adobe {
    SCWriter::SCWriter() :
        m_atlas(*this) {
    }

    SCWriter::~SCWriter() {
        if (fs::exists(sprite_temp_path)) {
            fs::remove(sprite_temp_path);
        }
    }

    void SCWriter::SetDocument(const std::vector<SymbolContext>& symbols) {
        SCPlugin& context = SCPlugin::Instance();

        m_status = context.Window()->CreateStatusBarComponent(context.locale.GetString("TID_BAR_LABEL_LIBRARY_ITEMS"));
        m_status->SetRange((int) symbols.size());
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
            isSc2 = flash::SupercellSWF::IsSC2(file);

            if (isSc2) {
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
                    config.generateStreamingTexture =
                        image->streaming_variants.has_value() && !image->streaming_variants.value().empty();
                }
            }

            for (auto& movieclip : swf.movieclips) {
                if (!movieclip.custom_properties.empty()) {
                    config.writeCustomProperties = true;
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

    void SCWriter::Finalize() {
        const SCConfig& config = SCPlugin::Publisher::ActiveConfig();
        SCPlugin& context = SCPlugin::Instance();

        if (swf.exports.empty())
            throw SCPluginException("TID_SWF_NO_EXPORTS", config.exportToExternalPath.string());

        enum FinalizeStep : uint8_t {
            INIT = 0,
            ATLAS_FINALIZE = 50,
            SWF_SAVING = 80,
            END = 100
        };

        StatusComponent* status =
            context.Window()->CreateStatusBarComponent(context.locale.GetString("TID_STATUS_INIT"), u"");

        if (config.exportToExternal) {
            flash::remove_unused(swf);
        }

        status->SetProgress(ATLAS_FINALIZE);
        status->SetStatusLabel(context.locale.GetString("TID_STATUS_TEXTURE_SAVE"));
        SetupTextureSettings();
        m_atlas.WriteAtlas();

        swf.use_precision_matrix = config.hasPrecisionMatrices;
        swf.save_custom_property = config.writeCustomProperties;

        // SC2 settings
        {
            flash::Sc2CompileSettings& sc2 = swf.sc2_compile_settings;
            sc2.use_half_precision_matrices = config.lowPrecisionMatrices;
            sc2.use_short_frames = config.useShortFrames;
        }

        fs::path filepath = fs::path(config.outputFilepath).replace_extension("sc");
        fs::path basename = filepath.filename();

        status->SetProgress(SWF_SAVING);
        status->SetStatusLabel(context.locale.GetString("TID_FILE_SAVE", basename.string()));

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

        context.Window()->DestroyStatusBar(status);
    }

    fs::path SCWriter::generateTempName() {
#if defined(_WINDOWS)
        return fs::path(tmpnam(nullptr));
#elif defined(__APPLE__)
        char name[] = "/tmp/swfSpriteXXXXXX";
        mkstemp(name);
        return fs::path(name);
#endif
    }

    void SCWriter::IncrementSymbolsProcessed() {
        if (!m_status)
            return;

        m_status->SetProgress((int) ++m_symbols_processed);
    }

    wk::RawImageRef SCWriter::GetBitmap(const BitmapElement& item) {
        const std::u16string& name = item.Name();

        auto it = m_cached_images.find(name);
        if (it != m_cached_images.end())
            return it->second;

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
