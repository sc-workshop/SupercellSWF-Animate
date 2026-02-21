#include "VectorRasterizer.h"

#include "Writer.h"

namespace sc::Adobe {
    void VectorRasterizer::CreateImage(wk::RawImageRef& image, sk_sp<SkSurface>& result, bool premultiply) {
        if (image->depth() != wk::Image::PixelDepth::RGBA8) {
            wk::RawImageRef converted =
                wk::CreateRef<wk::RawImage>(image->width(), image->height(), wk::Image::PixelDepth::RGBA8);

            image->copy(*converted);
            image = converted;
        }

        if (premultiply) {
            for (uint16_t h = 0; image->height() > h; h++) {
                for (uint16_t w = 0; image->width() > w; w++) {
                    wk::ColorRGBA& pixel = image->at<wk::ColorRGBA>(w, h);

                    float alpha = (float) pixel.a / 255.f;

                    pixel.r = (uint8_t) (pixel.r * alpha);
                    pixel.g = (uint8_t) (pixel.g * alpha);
                    pixel.b = (uint8_t) (pixel.b * alpha);
                }
            }
        }

        SkImageInfo info = SkImageInfo::MakeN32Premul(image->width(), image->height());
        size_t rowBytes = info.minRowBytes();
        result = SkSurfaces::WrapPixels(info, image->data(), rowBytes);
    }

    void VectorRasterizer::RoundBound(Animate::DOM::Utils::RECT& rect) {
        rect.bottomRight.x = std::round(rect.bottomRight.x);
        rect.bottomRight.y = std::round(rect.bottomRight.y);
        rect.topLeft.x = std::round(rect.topLeft.x);
        rect.topLeft.y = std::round(rect.topLeft.y);
    }

    void VectorRasterizer::CreatePath(const Animate::Publisher::FilledElementPath& path,
                                      SkPathBuilder& contour,
                                      float resolution) {
        uint8_t inited = false;
        for (size_t i = 0; path.Count() > i; i++) {
            const VectorSegment& segment = path.GetSegment(i);

            switch (segment.SegmentType()) {
                case VectorSegment::Type::Line: {
                    const auto& seg = (const VectorLineSegment&) segment;

                    if (!inited++) {
                        contour.moveTo(seg.begin.x, seg.begin.y);
                    }

                    contour.lineTo(seg.end.x, seg.end.y);
                } break;
                case VectorSegment::Type::Cubic: {
                    const auto& seg = (const VectorCubicSegment&) segment;

                    if (!inited++) {
                        contour.moveTo(seg.begin.x, seg.begin.y);
                    }

                    contour.cubicTo(seg.control_l.x,
                                    seg.control_l.y,
                                    seg.control_r.x,
                                    seg.control_r.y,
                                    seg.end.x,
                                    seg.end.y);
                } break;
                case VectorSegment::Type::Quad: {
                    const auto& seg = (const VectorQuadSegment&) segment;

                    if (!inited++) {
                        contour.moveTo(seg.begin.x, seg.begin.y);
                    }

                    contour.quadTo(seg.control.x, seg.control.y, seg.end.x, seg.end.y);
                } break;
                default:
                    break;
            }
        }

        if (resolution != 1.0f) {
            SkMatrix matrix = SkMatrix::Scale(resolution, resolution);
            contour.transform(matrix);
        }

        contour.close();
    }

    void VectorRasterizer::Add(const VectorShape& shape, const VectorMatrix& matrix) {
        VectorShape& region = m_queue.emplace_back(shape);
        region.Transform(matrix);
    }

    bool VectorRasterizer::Empty() const {
        return m_queue.empty();
    }

    bool VectorRasterizer::GetImage(wk::RawImageRef& image, VectorMatrix& matrix, float resolution) {
        VectorBound bound;
        for (const VectorShape& region : m_queue) {
            bound = region.Bound(bound);
        }

        wk::PointF shape_offset;
        shape_offset.x = std::min(bound.topLeft.x, bound.bottomRight.x);
        shape_offset.y = std::min(bound.topLeft.y, bound.bottomRight.y);

        VectorRasterizer::RoundBound(bound);
        if (!CreateCanvas(bound, resolution))
            return false;

        for (const VectorShape& region : m_queue) {
            VectorBound region_bound = region.Bound();

            wk::PointF region_offset;
            region_offset.x = std::min(region_bound.topLeft.x, region_bound.bottomRight.x);
            region_offset.y = std::min(region_bound.topLeft.y, region_bound.bottomRight.y);

            region_offset.x = -region_offset.x + (region_offset.x - shape_offset.x);
            region_offset.y = -region_offset.y + (region_offset.y - shape_offset.y);

            DrawRegion(region, region_offset, resolution);
        }

        ReleaseCanvas();
        image = std::move(m_image);
        matrix.a = 1.f / resolution;
        matrix.d = 1.f / resolution;
        matrix.tx = shape_offset.x / matrix.a;
        matrix.ty = shape_offset.y / matrix.d;

        m_queue.clear();
        return true;
    }

    bool VectorRasterizer::CreateCanvas(const Animate::DOM::Utils::RECT bound, float resolution) {
        uint16_t width = std::ceil(bound.topLeft.x - bound.bottomRight.x) * resolution;
        uint16_t height = std::ceil(bound.topLeft.y - bound.bottomRight.y) * resolution;
        if (width == 0 || height == 0)
            return false;

        m_image =
            wk::CreateRef<wk::RawImage>(width, height, wk::Image::PixelDepth::RGBA8, wk::Image::ColorSpace::Linear);

        sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeSRGB();
        SkImageInfo info = SkImageInfo::Make(width, height, kRGBA_8888_SkColorType, kPremul_SkAlphaType, colorSpace);
        size_t rowBytes = info.minRowBytes();
        m_canvas = SkSurfaces::WrapPixels(info, m_image->data(), rowBytes);
        m_draw = m_canvas->getCanvas();

        return true;
    }

    void VectorRasterizer::ReleaseCanvas() {
        m_draw = nullptr;
        m_canvas.reset();
    }

    void VectorRasterizer::DrawRegion(const Animate::Publisher::FilledElementRegion& region,
                                      wk::PointF offset,
                                      float resolution) {
        using namespace Animate::DOM;
        SkPathBuilder region_path;

        VectorMatrix resolution_matrix = {resolution, 0.f, 0.f, resolution, 0.f, 0.f};

        // Add holes paths first
        for (const auto& hole : region.holes) {
            SkPathBuilder contour;
            VectorRasterizer::CreatePath(hole, contour, resolution);
            region_path.addPath(contour.detach());
        }

        // Add contour then
        {
            SkPathBuilder contour;
            VectorRasterizer::CreatePath(region.contour, contour, resolution);
            region_path.addPath(contour.detach());
        }

        region_path.transform(SkMatrix::Translate(offset.x * resolution, offset.y * resolution));

        // Creating fill style
        SkPaint paint;
        paint.setAntiAlias(true);
        {
            if (region.type == VectorRegion::ShapeType::SolidColor) {
                const auto& fill = std::get<VectorRegion::SolidFill>(region.style);

                paint.setColor(SkColorSetARGB(fill.color.alpha, fill.color.red, fill.color.green, fill.color.blue));
                paint.setStyle(SkPaint::kFill_Style);

            } else if (region.type == VectorRegion::ShapeType::Bitmap) {
                const auto& fill = std::get<VectorRegion::BitmapFill>(region.style);

                fill.bitmap.ExportImage(m_writer.sprite_temp_path);
                sk_sp<SkData> data = SkData::MakeFromFileName(m_writer.sprite_temp_path.string().c_str());
                if (!data)
                    return;

                SkCodec::Result decode_result;
                auto codec = SkPngDecoder::Decode(data, &decode_result);
                if (decode_result != SkCodec::kSuccess)
                    return;

                auto [bitmap, image_result] = codec->getImage();
                if (image_result != SkCodec::kSuccess)
                    return;

                SkTileMode mode = fill.is_clipped ? SkTileMode::kRepeat : SkTileMode::kDecal;
                SkSamplingOptions sampling(SkFilterMode::kLinear, SkMipmapMode::kNone);

                auto matrix = fill.bitmap.Transformation();
                matrix.a /= Animate::DOM::TWIPS_PER_PIXEL;
                matrix.b /= Animate::DOM::TWIPS_PER_PIXEL;
                matrix.c /= Animate::DOM::TWIPS_PER_PIXEL;
                matrix.d /= Animate::DOM::TWIPS_PER_PIXEL;
                matrix.tx += offset.x;
                matrix.ty += offset.y;

                matrix = matrix * resolution_matrix;

                SkMatrix pattern_matrix =
                    SkMatrix::MakeAll(matrix.a, matrix.b, matrix.tx, matrix.c, matrix.d, matrix.ty, 0, 0, 1.f);

                paint.setShader(bitmap->makeShader(mode, mode, sampling, pattern_matrix));

            } else if (region.type == VectorRegion::ShapeType::GradientColor) {
                const auto& fill = std::get<VectorRegion::GradientFill>(region.style);

                sk_sp<SkShader> shader;

                SkTileMode tile_mode = SkTileMode::kDecal;
                switch (fill.spread) {
                    case FillStyle::GradientSpread::GRADIENT_SPREAD_EXTEND:
                        tile_mode = SkTileMode::kClamp;
                        break;
                    case FillStyle::GradientSpread::GRADIENT_SPREAD_REFLECT:
                        tile_mode = SkTileMode::kMirror;
                        break;
                    case FillStyle::GradientSpread::GRADIENT_SPREAD_REPEAT:
                        tile_mode = SkTileMode::kRepeat;
                        break;
                    default:
                        break;
                }

                VectorMatrix matrix = fill.matrix;
                matrix.tx += offset.x;
                matrix.ty += offset.y;
                matrix = matrix * resolution_matrix;

                SkMatrix gradient_matrix =
                    SkMatrix::MakeAll(matrix.a, matrix.b, matrix.tx, matrix.c, matrix.d, matrix.ty, 0.f, 0.f, 1.f);

                std::vector<SkColor4f> colors;
                colors.reserve(fill.points.size());

                std::vector<SkScalar> positions;
                positions.reserve(fill.points.size());

                for (auto& point : fill.points) {
                    colors.emplace_back(SkColor4f::FromColor(
                        SkColorSetARGB(point.color.alpha, point.color.red, point.color.green, point.color.blue)));

                    positions.push_back((float) point.pos / 0xFF);
                };

                SkGradient::Colors gradient_colors(SkSpan<const SkColor4f>(colors),
                                                   SkSpan<const float>(positions),
                                                   tile_mode);
                SkGradient::Interpolation interp;
                SkGradient gradient(gradient_colors, interp);

                switch (fill.type) {
                    case VectorRegion::GradientFill::FillType::Linear: {
                        const SkPoint points[2] = {SkPoint::Make(-0x333, 0), SkPoint::Make(0x333, 0)};
                        shader = SkShaders::LinearGradient(points, gradient, &gradient_matrix);
                    } break;
                    case VectorRegion::GradientFill::FillType::Radial: {
                        shader = SkShaders::RadialGradient(SkPoint::Make(0.f, 0.f), 0x333, gradient, &gradient_matrix);
                    } break;
                    default:
                        return;
                }

                paint.setShader(shader);
            }
        }

        m_draw->drawPath(region_path.detach(), paint);
    }
}