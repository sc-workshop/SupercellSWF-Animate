#include "SymbolCache.h"

#include "Cache/CacheManager.h"
#include "Module/Module.h"
#include "Writer/GraphicItem/FilledItem.h"
#include "Writer/GraphicItem/SlicedItem.h"
#include "Writer/GraphicItem/SpriteItem.h"
#include "Writer/MovieclipWriter.h"
#include "Writer/ShapeWriter.h"
#include "Writer/TextFieldWriter.h"
#include "Writer/Writer.h"
#include "core/exception/exception.h"
#include "core/io/memory_stream.h"
#include "core/memory/ref.h"
#include "core/string/string_converter.h"

#include <algorithm>
#include <any>
#include <atomic>
#include <chrono>
#include <cstring>
#include <format>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sc::Adobe::Cache {
    namespace {
        constexpr uint32_t PayloadMagic = 0x31435353; // "SCS1"

        enum class ShapeItemKind : uint8_t {
            Bitmap = 1,
            Sliced = 2,
            Filled = 3
        };

        class BinaryWriter {
        public:
            BinaryWriter() :
                m_stream(wk::CreateRef<wk::MemoryStream>(1024)) {}

            void Write(const void* data, size_t size) {
                Ensure(size);
                if (size != 0)
                    m_stream->write(data, size);
            }

            template <typename T>
            void WriteScalar(const T& value) {
                Write(&value, sizeof(value));
            }

            void WriteString(const std::string& value) {
                WriteScalar(static_cast<uint32_t>(value.size()));
                Write(value.data(), value.size());
            }

            void WriteU16String(const std::u16string& value) { WriteString(wk::StringConverter::ToUTF8(value)); }

            std::vector<std::byte> Finish() const {
                const auto* bytes = reinterpret_cast<const std::byte*>(m_stream->data());
                return std::vector<std::byte>(bytes, bytes + m_stream->position());
            }

        private:
            void Ensure(size_t size) {
                const size_t required = m_stream->position() + size;
                if (required <= m_stream->length())
                    return;

                size_t capacity = std::max<size_t>(m_stream->length() * 2, 1024);
                while (capacity < required)
                    capacity *= 2;

                auto next = wk::CreateRef<wk::MemoryStream>(capacity);
                if (m_stream->length() != 0) {
                    std::memcpy(next->data(), m_stream->data(), m_stream->length());
                }
                next->seek(m_stream->position());
                m_stream = next;
            }

        private:
            wk::Ref<wk::MemoryStream> m_stream;
        };

        class BinaryReader {
        public:
            explicit BinaryReader(const std::vector<std::byte>& data) :
                m_stream(wk::CreateRef<wk::MemoryStream>(data.size())) {
                if (!data.empty()) {
                    std::memcpy(m_stream->data(), data.data(), data.size());
                }
            }

            void Read(void* data, size_t size) {
                const size_t position = m_stream->position();
                if (m_stream->read(data, size) != size) {
                    throw wk::Exception(
                        std::format("truncated symbol cache payload at offset {} size {} of {}",
                                    position,
                                    size,
                                    m_stream->length()));
                }
            }

            template <typename T>
            T ReadScalar() {
                T value {};
                Read(&value, sizeof(value));
                return value;
            }

            std::string ReadString() {
                const uint32_t size = ReadScalar<uint32_t>();
                std::string result(size, '\0');
                if (size != 0)
                    Read(result.data(), result.size());
                return result;
            }

            std::u16string ReadU16String() { return wk::StringConverter::ToUTF16(ReadString()); }

        private:
            wk::Ref<wk::MemoryStream> m_stream;
        };

        ShapeItemKind ReadShapeItemKind(BinaryReader& reader) {
            const uint8_t raw = reader.ReadScalar<uint8_t>();
            switch (static_cast<ShapeItemKind>(raw)) {
                case ShapeItemKind::Bitmap:
                case ShapeItemKind::Sliced:
                case ShapeItemKind::Filled:
                    return static_cast<ShapeItemKind>(raw);
                default:
                    throw wk::Exception(std::format("unknown symbol cache graphic item kind={}", raw));
            }
        }

        void WriteMatrix(BinaryWriter& writer, const flash::Matrix2D& matrix) {
            writer.WriteScalar(matrix.a);
            writer.WriteScalar(matrix.b);
            writer.WriteScalar(matrix.c);
            writer.WriteScalar(matrix.d);
            writer.WriteScalar(matrix.tx);
            writer.WriteScalar(matrix.ty);
        }

        void WriteMatrix(BinaryWriter& writer, const Animate::DOM::Utils::MATRIX2D& matrix) {
            writer.WriteScalar(matrix.a);
            writer.WriteScalar(matrix.b);
            writer.WriteScalar(matrix.c);
            writer.WriteScalar(matrix.d);
            writer.WriteScalar(matrix.tx);
            writer.WriteScalar(matrix.ty);
        }

        flash::Matrix2D ReadMatrix(BinaryReader& reader) {
            flash::Matrix2D matrix;
            matrix.a = reader.ReadScalar<float>();
            matrix.b = reader.ReadScalar<float>();
            matrix.c = reader.ReadScalar<float>();
            matrix.d = reader.ReadScalar<float>();
            matrix.tx = reader.ReadScalar<float>();
            matrix.ty = reader.ReadScalar<float>();
            return matrix;
        }

        void WriteColorTransform(BinaryWriter& writer, const flash::ColorTransform& color) {
            writer.WriteScalar(color.alpha);
            writer.WriteScalar(color.add.r);
            writer.WriteScalar(color.add.g);
            writer.WriteScalar(color.add.b);
            writer.WriteScalar(color.multiply.r);
            writer.WriteScalar(color.multiply.g);
            writer.WriteScalar(color.multiply.b);
        }

        flash::ColorTransform ReadColorTransform(BinaryReader& reader) {
            flash::ColorTransform color;
            color.alpha = reader.ReadScalar<uint8_t>();
            color.add.r = reader.ReadScalar<uint8_t>();
            color.add.g = reader.ReadScalar<uint8_t>();
            color.add.b = reader.ReadScalar<uint8_t>();
            color.multiply.r = reader.ReadScalar<uint8_t>();
            color.multiply.g = reader.ReadScalar<uint8_t>();
            color.multiply.b = reader.ReadScalar<uint8_t>();
            return color;
        }

        void WriteRect(BinaryWriter& writer, const wk::RectF& rect) {
            writer.WriteScalar(rect.left);
            writer.WriteScalar(rect.top);
            writer.WriteScalar(rect.right);
            writer.WriteScalar(rect.bottom);
        }

        void WriteRect(BinaryWriter& writer, const Animate::DOM::Utils::RECT& rect) {
            writer.WriteScalar(rect.topLeft.x);
            writer.WriteScalar(rect.topLeft.y);
            writer.WriteScalar(rect.bottomRight.x);
            writer.WriteScalar(rect.bottomRight.y);
        }

        wk::RectF ReadRect(BinaryReader& reader) {
            wk::RectF rect;
            rect.left = reader.ReadScalar<float>();
            rect.top = reader.ReadScalar<float>();
            rect.right = reader.ReadScalar<float>();
            rect.bottom = reader.ReadScalar<float>();
            return rect;
        }

        void WriteColor(BinaryWriter& writer, const wk::ColorRGBA& color) {
            writer.WriteScalar(color.r);
            writer.WriteScalar(color.g);
            writer.WriteScalar(color.b);
            writer.WriteScalar(color.a);
        }

        wk::ColorRGBA ReadColor(BinaryReader& reader) {
            wk::ColorRGBA color;
            color.r = reader.ReadScalar<uint8_t>();
            color.g = reader.ReadScalar<uint8_t>();
            color.b = reader.ReadScalar<uint8_t>();
            color.a = reader.ReadScalar<uint8_t>();
            return color;
        }

        void WriteImage(BinaryWriter& writer, const wk::RawImage& image) {
            writer.WriteScalar(image.width());
            writer.WriteScalar(image.height());
            writer.WriteScalar(static_cast<uint8_t>(image.depth()));
            writer.WriteScalar(static_cast<uint8_t>(image.colorspace()));
            const size_t length = image.data_length();
            writer.WriteScalar(static_cast<uint32_t>(length));
            writer.Write(image.data(), length);
        }

        wk::RawImageRef ReadImage(BinaryReader& reader) {
            const uint16_t width = reader.ReadScalar<uint16_t>();
            const uint16_t height = reader.ReadScalar<uint16_t>();
            const auto depth = static_cast<wk::Image::PixelDepth>(reader.ReadScalar<uint8_t>());
            const auto space = static_cast<wk::Image::ColorSpace>(reader.ReadScalar<uint8_t>());
            const uint32_t length = reader.ReadScalar<uint32_t>();

            auto image = wk::CreateRef<wk::RawImage>(width, height, depth, space);
            if (image->data_length() != length)
                throw wk::Exception("symbol cache image size mismatch");

            if (length != 0)
                reader.Read(image->data(), length);

            return image;
        }

        void WriteSymbolContext(BinaryWriter& writer, const Animate::Publisher::SymbolContext& context) {
            writer.WriteU16String(context.name);
            writer.WriteString(context.linkage_name);
            writer.WriteScalar(static_cast<uint8_t>(context.type));
        }

        Animate::Publisher::SymbolContext ReadSymbolContext(BinaryReader& reader) {
            // Field order must match WriteSymbolContext: name, linkage, type.
            const std::u16string name = reader.ReadU16String();
            const std::string linkage = reader.ReadString();
            const auto type = static_cast<Animate::Publisher::SymbolContext::SymbolType>(reader.ReadScalar<uint8_t>());
            Animate::Publisher::SymbolContext context(name, type);
            context.linkage_name = linkage;
            return context;
        }
    }

    std::vector<std::byte> SymbolCache::StoreMovieClip(const SCMovieclipWriter& writer) {
        BinaryWriter stream;
        stream.WriteScalar(PayloadMagic);
        stream.WriteScalar(writer.m_frameCount);
        stream.WriteScalar(writer.m_object.bank_index);
        stream.WriteScalar(writer.m_object.frame_rate);
        stream.WriteScalar(static_cast<uint8_t>(writer.m_object.unknown_flag));
        stream.WriteScalar(writer.m_object.scaling_grid.has_value());
        if (writer.m_object.scaling_grid)
            WriteRect(stream, *writer.m_object.scaling_grid);

        stream.WriteScalar(static_cast<uint32_t>(writer.m_object.frames.size()));
        for (const flash::MovieClipFrame& frame : writer.m_object.frames) {
            stream.WriteString(frame.label.string());
            stream.WriteScalar(frame.elements_count);
        }

        stream.WriteScalar(static_cast<uint32_t>(writer.m_object.childrens.size()));
        for (const flash::DisplayObjectInstance& child : writer.m_object.childrens) {
            stream.WriteScalar(child.id);
            stream.WriteScalar(static_cast<uint8_t>(child.blend_mode));
            stream.WriteString(child.name.string());
        }

        stream.WriteScalar(static_cast<uint32_t>(writer.m_object.frame_elements.size()));
        for (const flash::MovieClipFrameElement& element : writer.m_object.frame_elements) {
            stream.WriteScalar(element.instance_index);
            stream.WriteScalar(element.matrix_index);
            stream.WriteScalar(element.colorTransform_index);
        }

        stream.WriteScalar(static_cast<uint32_t>(writer.m_bank.matrices.size()));
        for (const flash::Matrix2D& matrix : writer.m_bank.matrices)
            WriteMatrix(stream, matrix);

        stream.WriteScalar(static_cast<uint32_t>(writer.m_bank.color_transforms.size()));
        for (const flash::ColorTransform& color : writer.m_bank.color_transforms)
            WriteColorTransform(stream, color);

        stream.WriteScalar(static_cast<uint32_t>(writer.m_bank.movieclip_elements.size()));
        if (!writer.m_bank.movieclip_elements.empty()) {
            stream.Write(writer.m_bank.movieclip_elements.data(), writer.m_bank.movieclip_elements.size());
        }

        // Custom properties are opaque game flags. Only bool is representable in the
        // SWF format; reject anything else instead of silently dropping it.
        const size_t propertyCount = writer.m_object.custom_properties.size();
        if (propertyCount > 0xFF)
            return {};
        stream.WriteScalar(static_cast<uint8_t>(propertyCount));
        for (const std::any& property : writer.m_object.custom_properties) {
            const bool* value = std::any_cast<bool>(&property);
            if (!value)
                return {};
            stream.WriteScalar(static_cast<uint8_t>(*value ? 1 : 0));
        }

        return stream.Finish();
    }

    void SymbolCache::RestoreMovieClip(SCMovieclipWriter& writer, const std::vector<std::byte>& data) {
        BinaryReader stream(data);
        if (stream.ReadScalar<uint32_t>() != PayloadMagic)
            throw wk::Exception("unsupported symbol cache payload");

        writer.m_frameCount = stream.ReadScalar<uint32_t>();
        writer.m_object.bank_index = stream.ReadScalar<uint32_t>();
        writer.m_object.frame_rate = stream.ReadScalar<uint8_t>();
        writer.m_object.unknown_flag = stream.ReadScalar<uint8_t>() != 0;
        if (stream.ReadScalar<uint8_t>() != 0)
            writer.m_object.scaling_grid = ReadRect(stream);
        else
            writer.m_object.scaling_grid.reset();

        const uint32_t frameCount = stream.ReadScalar<uint32_t>();
        writer.m_object.frames.clear();
        writer.m_object.frames.reserve(frameCount);
        for (uint32_t i = 0; i < frameCount; ++i) {
            auto& frame = writer.m_object.frames.emplace_back();
            frame.label = flash::SWFString(stream.ReadString());
            frame.elements_count = stream.ReadScalar<uint32_t>();
        }

        const uint32_t childCount = stream.ReadScalar<uint32_t>();
        writer.m_object.childrens.clear();
        writer.m_object.childrens.reserve(childCount);
        for (uint32_t i = 0; i < childCount; ++i) {
            auto& child = writer.m_object.childrens.emplace_back();
            child.id = stream.ReadScalar<uint16_t>();
            child.blend_mode = static_cast<flash::DisplayObjectInstance::BlendMode>(stream.ReadScalar<uint8_t>());
            child.name = flash::SWFString(stream.ReadString());
        }

        const uint32_t elementCount = stream.ReadScalar<uint32_t>();
        writer.m_object.frame_elements.clear();
        writer.m_object.frame_elements.reserve(elementCount);
        for (uint32_t i = 0; i < elementCount; ++i) {
            auto& element = writer.m_object.frame_elements.emplace_back();
            element.instance_index = stream.ReadScalar<uint16_t>();
            element.matrix_index = stream.ReadScalar<uint16_t>();
            element.colorTransform_index = stream.ReadScalar<uint16_t>();
        }

        const uint32_t matrixCount = stream.ReadScalar<uint32_t>();
        writer.m_bank.matrices.clear();
        writer.m_bank.matrices.reserve(matrixCount);
        for (uint32_t i = 0; i < matrixCount; ++i)
            writer.m_bank.matrices.emplace_back(ReadMatrix(stream));

        const uint32_t colorCount = stream.ReadScalar<uint32_t>();
        writer.m_bank.color_transforms.clear();
        writer.m_bank.color_transforms.reserve(colorCount);
        for (uint32_t i = 0; i < colorCount; ++i)
            writer.m_bank.color_transforms.emplace_back(ReadColorTransform(stream));

        const uint32_t extraCount = stream.ReadScalar<uint32_t>();
        writer.m_bank.movieclip_elements.clear();
        writer.m_bank.movieclip_elements.reserve(extraCount);
        for (uint32_t i = 0; i < extraCount; ++i)
            writer.m_bank.movieclip_elements.emplace_back(stream.ReadScalar<uint8_t>());

        const uint8_t propertyCount = stream.ReadScalar<uint8_t>();
        writer.m_object.custom_properties.clear();
        writer.m_object.custom_properties.reserve(propertyCount);
        for (uint8_t i = 0; i < propertyCount; ++i)
            writer.m_object.custom_properties.emplace_back(std::make_any<bool>(stream.ReadScalar<uint8_t>() != 0));
    }

    std::vector<std::byte> SymbolCache::StoreShape(const SCShapeWriter& writer) {
        BinaryWriter stream;

        stream.WriteScalar(PayloadMagic);
        stream.WriteScalar(static_cast<uint32_t>(writer.m_group.Size()));
        stream.WriteScalar(writer.m_guides.has_value());
        if (writer.m_guides)
            WriteRect(stream, *writer.m_guides);

        for (size_t i = 0; writer.m_group.Size() > i; ++i) {
            const Animate::Publisher::StaticElement& base = writer.m_group[i];
            WriteSymbolContext(stream, base.Symbol());
            WriteMatrix(stream, base.Transformation());

            const auto& item = static_cast<const GraphicItem&>(base);
            if (item.Is9Sliced()) {
                const auto& sliced = static_cast<const SlicedItem&>(item);
                stream.WriteScalar(static_cast<uint8_t>(ShapeItemKind::Sliced));
                WriteImage(stream, sliced.Image());
                stream.WriteScalar(sliced.IsRasterizedVector());
                stream.WriteScalar(sliced.Translation().x);
                stream.WriteScalar(sliced.Translation().y);
                WriteRect(stream, sliced.Guides());
            } else if (item.IsSprite()) {
                const auto& sprite = static_cast<const BitmapItem&>(item);
                stream.WriteScalar(static_cast<uint8_t>(ShapeItemKind::Bitmap));
                WriteImage(stream, sprite.Image());
                stream.WriteScalar(sprite.IsRasterizedVector());
            } else {
                const auto& filled = static_cast<const FilledItem&>(item);
                stream.WriteScalar(static_cast<uint8_t>(ShapeItemKind::Filled));
                WriteColor(stream, filled.Color());
                stream.WriteScalar(static_cast<uint32_t>(filled.contours.size()));
                for (const FilledItemContour& contour : filled.contours) {
                    stream.WriteScalar(static_cast<uint32_t>(contour.Contour().size()));
                    for (const Animate::Publisher::Point2D& point : contour.Contour()) {
                        stream.WriteScalar(point.x);
                        stream.WriteScalar(point.y);
                    }
                }
            }
        }

        return stream.Finish();
    }

    void SymbolCache::RestoreShape(SCShapeWriter& writer, const std::vector<std::byte>& data) {
        BinaryReader stream(data);
        if (stream.ReadScalar<uint32_t>() != PayloadMagic)
            throw wk::Exception("unsupported symbol cache payload");

        const uint32_t itemCount = stream.ReadScalar<uint32_t>();
        writer.m_group.Clear();
        if (stream.ReadScalar<uint8_t>() != 0) {
            const wk::RectF guides = ReadRect(stream);
            writer.m_guides = Animate::DOM::Utils::RECT {{guides.left, guides.top}, {guides.right, guides.bottom}};
        } else {
            writer.m_guides.reset();
        }

        for (uint32_t i = 0; i < itemCount; ++i) {
            Animate::Publisher::SymbolContext context = ReadSymbolContext(stream);
            const flash::Matrix2D matrix = ReadMatrix(stream);
            Animate::DOM::Utils::MATRIX2D domMatrix = {matrix.a, matrix.b, matrix.c, matrix.d, matrix.tx, matrix.ty};
            const ShapeItemKind kind = ReadShapeItemKind(stream);

            if (kind == ShapeItemKind::Bitmap || kind == ShapeItemKind::Sliced) {
                wk::RawImageRef image = ReadImage(stream);
                const bool rasterized = stream.ReadScalar<uint8_t>() != 0;

                if (kind == ShapeItemKind::Sliced) {
                    const float translationX = stream.ReadScalar<float>();
                    const float translationY = stream.ReadScalar<float>();
                    const wk::RectF guides = ReadRect(stream);
                    const Animate::DOM::Utils::RECT domGuides = {{guides.left, guides.top},
                                                                 {guides.right, guides.bottom}};
                    writer.m_group.AddElement<SlicedItem>(context,
                                                          image,
                                                          domMatrix,
                                                          wk::PointF(translationX, translationY),
                                                          domGuides);
                } else {
                    writer.m_group.AddElement<BitmapItem>(context, image, domMatrix, rasterized);
                }
                continue;
            }

            const wk::ColorRGBA color = ReadColor(stream);
            const Animate::DOM::Utils::COLOR domColor = {color.r, color.g, color.b, color.a};
            const uint32_t contourCount = stream.ReadScalar<uint32_t>();
            std::vector<FilledItemContour> contours;
            contours.reserve(contourCount);
            for (uint32_t contourIndex = 0; contourIndex < contourCount; ++contourIndex) {
                const uint32_t pointCount = stream.ReadScalar<uint32_t>();
                std::vector<Animate::Publisher::Point2D> points;
                points.reserve(pointCount);
                for (uint32_t pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
                    Animate::Publisher::Point2D point;
                    point.x = stream.ReadScalar<float>();
                    point.y = stream.ReadScalar<float>();
                    points.push_back(point);
                }
                contours.emplace_back(points);
            }

            writer.m_group.AddElement<FilledItem>(context, contours, domColor, domMatrix);
        }
    }

    std::vector<std::byte> SymbolCache::StoreTextField(const SCTextFieldWriter& writer) {
        BinaryWriter stream;
        stream.WriteScalar(PayloadMagic);
        stream.WriteString(writer.m_object.text.string());
        stream.WriteString(writer.m_object.font_name.string());
        WriteColor(stream, writer.m_object.font_color);
        stream.WriteScalar(writer.m_object.font_size);
        stream.WriteScalar(static_cast<uint8_t>(writer.m_object.font_horizontal_align));
        stream.WriteScalar(static_cast<uint8_t>(writer.m_object.font_vertical_align));
        stream.WriteScalar(writer.m_object.unknown_align6);
        stream.WriteScalar(writer.m_object.unknown_align7);
        stream.WriteScalar(writer.m_object.left);
        stream.WriteScalar(writer.m_object.top);
        stream.WriteScalar(writer.m_object.right);
        stream.WriteScalar(writer.m_object.bottom);
        stream.WriteScalar(writer.m_object.style.flags());
        WriteColor(stream, writer.m_object.outline_color);
        stream.WriteScalar(writer.m_object.bend_angle);
        stream.WriteScalar(writer.m_object.outline_angle);
        stream.WriteString(writer.m_object.typography_file.string());
        return stream.Finish();
    }

    void SymbolCache::RestoreTextField(SCTextFieldWriter& writer, const std::vector<std::byte>& data) {
        BinaryReader stream(data);
        if (stream.ReadScalar<uint32_t>() != PayloadMagic)
            throw wk::Exception("unsupported symbol cache payload");

        writer.m_object.text = flash::SWFString(stream.ReadString());
        writer.m_object.font_name = flash::SWFString(stream.ReadString());
        writer.m_object.font_color = ReadColor(stream);
        writer.m_object.font_size = stream.ReadScalar<uint8_t>();
        writer.m_object.font_horizontal_align = static_cast<flash::TextField::Align>(stream.ReadScalar<uint8_t>());
        writer.m_object.font_vertical_align = static_cast<flash::TextField::Align>(stream.ReadScalar<uint8_t>());
        writer.m_object.unknown_align6 = stream.ReadScalar<uint8_t>() != 0;
        writer.m_object.unknown_align7 = stream.ReadScalar<uint8_t>() != 0;
        writer.m_object.left = stream.ReadScalar<int16_t>();
        writer.m_object.top = stream.ReadScalar<int16_t>();
        writer.m_object.right = stream.ReadScalar<int16_t>();
        writer.m_object.bottom = stream.ReadScalar<int16_t>();
        writer.m_object.style = flash::TextFieldStyle(stream.ReadScalar<uint8_t>());
        writer.m_object.outline_color = ReadColor(stream);
        writer.m_object.bend_angle = stream.ReadScalar<float>();
        writer.m_object.outline_angle = stream.ReadScalar<int32_t>();
        writer.m_object.typography_file = flash::SWFString(stream.ReadString());
    }

    Cache::CacheKey ToCacheKey(const Animate::Publisher::SymbolCacheKey& key) {
        Cache::CacheKey result {};
        std::copy(key.begin(), key.end(), result.begin());
        return result;
    }

    Animate::Publisher::SymbolCacheKey FromCacheKey(const Cache::CacheKey& key) {
        Animate::Publisher::SymbolCacheKey result {};
        std::copy(key.begin(), key.end(), result.begin());
        return result;
    }

    SymbolCache::SymbolCache(CacheManager& manager, SCWriter& writer) :
        m_manager(manager),
        m_writer(writer) {
    }

    bool SymbolCache::IsEnabled() const {
        return m_manager.IsEnabled();
    }

    Animate::Publisher::SymbolCacheKey SymbolCache::KeyForSymbol(
        const Animate::Publisher::SymbolContext& symbol) const {
        Cache::CacheKey key {};
        if (!m_manager.GetSymbolKey(symbol, key))
            return {};
        return FromCacheKey(key);
    }

    Animate::Publisher::SymbolCacheKey SymbolCache::DeriveKey(const Animate::Publisher::SymbolCacheKey& owner,
                                                              uint32_t ordinal) const {
        CacheKeyBuilder builder;
        builder.update(owner.data(), owner.size());
        builder.updateScalar(ordinal);
        return FromCacheKey(builder.finish());
    }

    bool SymbolCache::TryLoad(const Animate::Publisher::SymbolCacheKey& key,
                              Animate::Publisher::CachedSymbolPayload& payload) const {
        return m_manager.TryLoadSymbol(ToCacheKey(key), payload);
    }

    bool SymbolCache::Store(const Animate::Publisher::SymbolCacheKey& key,
                            const Animate::Publisher::CachedSymbolPayload& payload) {
        return m_manager.StoreSymbol(ToCacheKey(key), payload);
    }

    bool SymbolCache::RestoreWriter(const Animate::Publisher::CachedSymbolPayload& payload,
                                    Animate::Publisher::SymbolContext& context,
                                    wk::Ref<Animate::Publisher::IDisplayObjectWriter>& writer) const {
        const auto started = std::chrono::steady_clock::now();
        try {
            using Animate::Publisher::WriterType;
            switch (payload.type) {
                case WriterType::MovieClip: {
                    auto result = wk::CreateRef<SCMovieclipWriter>(m_writer, context);
                    RestoreMovieClip(*result, payload.data);
                    writer = result;
                    return true;
                }

                case WriterType::TextField: {
                    auto result = wk::CreateRef<SCTextFieldWriter>(m_writer, context);
                    RestoreTextField(*result, payload.data);
                    writer = result;
                    return true;
                }

                case WriterType::Graphic:
                default: {
                    auto result = wk::CreateRef<SCShapeWriter>(m_writer, context);
                    RestoreShape(*result, payload.data);
                    writer = result;
                    return true;
                }
            }
        } catch (const std::exception& error) {
            static std::atomic<uint32_t> logged {0};
            if (logged.fetch_add(1) < 20) {
                const auto elapsed =
                    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - started)
                        .count();
                SCPlugin& contextDiag = SCPlugin::Instance();
                contextDiag.logger->warn("[Perf] RestoreWriter failed type={} dataSize={} ms={} error={}",
                                         static_cast<int>(payload.type),
                                         payload.data.size(),
                                         elapsed,
                                         error.what());
            }
            return false;
        } catch (...) {
            static std::atomic<uint32_t> loggedUnknown {0};
            if (loggedUnknown.fetch_add(1) < 20) {
                SCPlugin& contextDiag = SCPlugin::Instance();
                contextDiag.logger->warn("[Perf] RestoreWriter failed unknown type={} dataSize={} ms={}",
                                         static_cast<int>(payload.type),
                                         payload.data.size(),
                                         std::chrono::duration_cast<std::chrono::milliseconds>(
                                             std::chrono::steady_clock::now() - started)
                                             .count());
            }
            return false;
        }
    }

    bool SymbolCache::CaptureWriter(const Animate::Publisher::IDisplayObjectWriter& writer,
                                    Animate::Publisher::CachedSymbolPayload& payload) const {
        auto rejected = [&](const char* reason) {
            static std::atomic<uint32_t> logged {0};
            if (logged.fetch_add(1) < 20) {
                SCPlugin& plugin = SCPlugin::Instance();
                plugin.logger->warn("[Cache] capture-rejected type={} name={} reason={}",
                                    static_cast<int>(payload.type),
                                    wk::StringConverter::ToUTF8(payload.name),
                                    reason);
            }
            return false;
        };

        try {
            using Animate::Publisher::WriterType;
            switch (writer.Type()) {
                case WriterType::MovieClip:
                    payload.data = StoreMovieClip(static_cast<const SCMovieclipWriter&>(writer));
                    return !payload.data.empty() || rejected("empty-movieclip");

                case WriterType::TextField:
                    payload.data = StoreTextField(static_cast<const SCTextFieldWriter&>(writer));
                    return !payload.data.empty() || rejected("empty-textfield");

                case WriterType::Graphic:
                default:
                    payload.data = StoreShape(static_cast<const SCShapeWriter&>(writer));
                    return !payload.data.empty() || rejected("empty-shape");
            }
        } catch (...) {
            return rejected("exception");
        }
    }
}
